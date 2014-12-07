#ifndef PROPERTY_H
#define PROPERTY_H

#define DECLARE_PROPERTY(T,X)\
    public:\
        T get_##X() const { return m_##X; }\
        void set_##X(const T& X) { m_##X = X; }\
        void set_##X(T&& X) { m_##X = std::move( X ); }\
    private:\
        T m_##X;

#endif // PROPERTY_H
