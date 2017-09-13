#include "util.h"

template<typename TPin, size_t Position>
struct PinWrapper {
    typedef TPin Pin;
    enum { pos = Position };
};

template <typename...>
struct BitMask {
    enum { value = 0 };
};

template <typename T, typename... Ts>
struct BitMask<TList<T, Ts...> > {
    enum { value = (1 << T::Pin::number) | BitMask<TList<Ts...> >::value };
};

template <size_t, typename...>
struct GetPinsWithPort {
    typedef TList<> value;
};

template <size_t Position, typename Port, typename... Ts, size_t Number>
struct GetPinsWithPort<Position, Port, TPin<Port, Number>, Ts...> {
    typedef typename GetPinsWithPort<Position + 1, Port, Ts...>::value tail;
    typedef typename FromList<PinWrapper<TPin<Port, Number>, Position>, tail>::value value;
};

template <size_t Position, typename Port, typename T, typename... Ts>
struct GetPinsWithPort<Position, Port, T, Ts...> {
    typedef typename GetPinsWithPort<Position + 1, Port, Ts...>::value value;
};

template <typename...>
struct IsSerial;

template <typename T>
struct IsSerial<T> {
    enum { value = 1 };
    enum { headNumber = T::Pin::number };
};

template <typename T, typename... Ts>
struct IsSerial<T, Ts...> {
    typedef IsSerial<Ts...> tail;

    enum { headNumber = T::Pin::number };
    enum { value = (tail::headNumber == headNumber + 1) && tail::value };
};

template <typename...>
struct PinMap {
    template<class R, class T>
    static R toValue(const T &value) {
        return 0;
    }

    template<class R, class T>
    static R fromValue(const T &value) {
        return 0;
    }
};

template <typename W, typename... Ws>
struct PinMap<TList<W, Ws...> > {
    typedef TList<W, Ws...> PinsOfPort;
    enum { mask = BitMask<PinsOfPort>::value };

    template<class R, class T>
    static inline R map(size_t from, size_t to, const T &value) {
        R result = 0;

        if (from == to)
            result |= value & (1 << from);
        else
            if (value & (1 << from))
                result |= (1 << to);

        return result;
    }

    template<class R, class T>
    static inline R toValue(const T &value) {
        auto pos = (size_t)W::pos;
        auto pin = (size_t)W::Pin::number;

        if (IsSerial<W, Ws...>::value) {
            if (pos > pin)
                return (value >> (pos - pin)) & mask;
            else
                return (value << (pin - pos)) & mask;
        }

        auto result = map<R>(pos, pin, value);
        return result | PinMap<TList<Ws...> >::template toValue<R>(value);
    }

    template<class R, class T>
    static inline R fromValue(const T &value) {
        auto pos = (size_t)W::pos;
        auto pin = (size_t)W::Pin::number;

        if (IsSerial<W, Ws...>::value) {
            if (pos > pin)
                return (value & mask) << (pos - pin);
            else
                return (value & mask) >> (pin - pos);
        }

        auto result = map<R>(pin, pos, value);
        return result | PinMap<TList<Ws...> >::template fromValue<R>(value);
    }
};

template <typename...>
struct PortWriter {
    template<class R>
    static R read() {
        return 0;
    }

    template<class T>
    static void dirWrite(T value) {}

    template<class T>
    static void write(T value) {}
};

template <typename Port, typename... Ports, typename... AllPins>
struct PortWriter<TList<Port, Ports...>, AllPins...> {
    typedef typename GetPinsWithPort<0, Port, AllPins...>::value PinsOfPort;
    enum { mask = BitMask<PinsOfPort>::value };

    template<class R>
    static R read() {
        auto portData = Port::read();
        auto value = PinMap<PinsOfPort>::template fromValue<R>(portData);
        return value | PortWriter<TList<Ports...>, AllPins...>::template read<R>();
    }

    template<class T>
    static void dirWrite(T value) {
        auto result = PinMap<PinsOfPort>::template toValue<typename Port::DataT>(value);

        if ((size_t)Length<PinsOfPort>::value == (size_t)Port::width)
            Port::dirWrite(result);
        else {
            Port::dirClear(mask);
            Port::dirSet(result);
        }

        PortWriter<TList<Ports...>, AllPins...>::dirWrite(value);
    }

    template<class T>
    static void write(T value) {
        auto result = PinMap<PinsOfPort>::template toValue<typename Port::DataT>(value);

        if ((size_t)Length<PinsOfPort>::value == (size_t)Port::width)
            Port::write(result);
        else
            Port::clearSet(mask, result);

        PortWriter<TList<Ports...>, AllPins...>::write(value);
    }
};

template<typename... AllPins>
struct VirtualPort {
    typedef typename NoDuplicates<typename AllPins::Port...>::value ports;

    template<class R>
    static R read() {
        return PortWriter<ports, AllPins...>::template read<R>();
    }

    template<class T>
    static void dirWrite(T value) {
        PortWriter<ports, AllPins...>::dirWrite(value);
    }

    template<class T>
    static void write(T value) {
        PortWriter<ports, AllPins...>::write(value);
    }

    template<class R>
    static void asOutput() {
        dirWrite((1 << sizeof(R)*8) - 1);
    }

    static void asInput() {
        dirWrite(0);
    }
};
