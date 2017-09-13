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
struct Length {
    enum { value = 0 };
};

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
