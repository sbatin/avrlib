//  Copyright (c) 2017 Sergey Batin. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef VIRTUALPORT_h
#define VIRTUALPORT_h

/*
 ********* generic helpers to work with variadic templates ************
 */

// just dummy container
template <typename...>
struct TList;

struct TTrue;
struct TFalse;

template <typename R, typename TList>
struct IsElem {
    typedef TFalse value;
};

template <typename R, typename... Ts>
struct IsElem<R, TList<R, Ts...> > {
    typedef TTrue value;
};

template <typename R, typename T, typename... Ts>
struct IsElem<R, TList<T, Ts...> > {
    typedef typename IsElem<R, TList<Ts...> >::value value;
};

template <typename...>
struct Length;

template <typename... Ts>
struct Length<TList<Ts...> > {
    enum { value = sizeof...(Ts) };
};

template <typename R, typename TList, typename T_has = typename IsElem<R, TList>::value>
struct AddUnique;

template <typename R, typename... Ts>
struct AddUnique<R, TList<Ts...>, TTrue> {
    typedef TList<Ts...> value;
};

template <typename R, typename... Ts>
struct AddUnique<R, TList<Ts...>, TFalse> {
    typedef TList<R, Ts...> value;
};

template <typename... Ts>
struct NoDuplicates {
    typedef TList<> value;
};

template <typename T, typename... Ts>
struct NoDuplicates<T, Ts...> {
    typedef typename AddUnique<T, typename NoDuplicates<Ts...>::value>::value value;
};

template <typename... Ts>
struct FromList {
    typedef TList<> value;
};

template <typename T, typename... Ts>
struct FromList<T, TList<Ts...> > {
    typedef TList<T, Ts...> value;
};

/*
 ********* virtual ports helpers ************
 */

template<typename TPin, int Position>
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

template <int, typename...>
struct GetPinsWithPort {
    typedef TList<> value;
};

template <int Position, typename Port, typename... Ts, int Number>
struct GetPinsWithPort<Position, Port, TPin<Port, Number>, Ts...> {
    typedef typename GetPinsWithPort<Position + 1, Port, Ts...>::value tail;
    typedef typename FromList<PinWrapper<TPin<Port, Number>, Position>, tail>::value value;
};

template <int Position, typename Port, typename T, typename... Ts>
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
    static inline R map(int from, int to, const T &value) {
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
        auto pos = (int)W::pos;
        auto pin = (int)W::Pin::number;

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
        auto pos = (int)W::pos;
        auto pin = (int)W::Pin::number;

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

        if ((int)Length<PinsOfPort>::value == (int)Port::width)
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

        if ((int)Length<PinsOfPort>::value == (int)Port::width)
            Port::write(result);
        else
            Port::clearSet(mask, result);

        PortWriter<TList<Ports...>, AllPins...>::write(value);
    }
};

/*
 ********* virtual ports main class ************
 */

template<typename... AllPins>
struct VirtualPort {
    typedef typename NoDuplicates<typename AllPins::Port...>::value ports;

    template<class R>
    static R read() {
        return PortWriter<ports, AllPins...>::template read<R>();
    }

    template<class T>
    static void toOutput(T value) {
        PortWriter<ports, AllPins...>::dirWrite(value);
    }

    template<class T>
    static void pullUp(T value) {
        toOutput(~value);
        write(value);
    }

    template<class T>
    static void write(T value) {
        PortWriter<ports, AllPins...>::write(value);
    }
};

#endif
