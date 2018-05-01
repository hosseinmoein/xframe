/***************************************************************************
* Copyright (c) 2017, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XFRAME_XAXIS_SLICE_HPP
#define XFRAME_XAXIS_SLICE_HPP

#include <cmath>
#include <xtl/xvariant.hpp>
#include "xtensor_xslice.hpp"
#include "xframe_config.hpp"

namespace xf
{

    /*************************
     * xaxis_extended_islice *
     *************************/

    template <class T>
    class xaxis_extended_islice;

    namespace detail
    {
        template <class T>
        struct is_xaxis_extended_islice : std::false_type
        {
        };

        template <class T>
        struct is_xaxis_extended_islice<xaxis_extended_islice<T>> : std::true_type
        {
        };

        template <class S>
        using disable_xaxis_extended_islice_t = std::enable_if_t<!is_xaxis_extended_islice<std::decay_t<S>>::value, void>;
    }

    template <class T>
    class xaxis_extended_islice
    {
    public:

        using self_type = xaxis_extended_islice<T>;
        using all_type = xt::xall_tag;
        using squeeze_type = T;
        using slice_type = xt::xslice_variant<T>;
        using storage_type = xtl::variant<all_type, squeeze_type, slice_type>;

        xaxis_extended_islice() = default;
        template <class S, typename = detail::disable_xaxis_extended_islice_t<S>>
        xaxis_extended_islice(S&& slice);

        template <class S, typename = std::enable_if_t<std::is_convertible<S, T>::value, void>>
        operator xaxis_extended_islice<S>() const noexcept;

        const all_type* get_all() const noexcept;
        const squeeze_type* get_squeeze() const noexcept;
        const slice_type* get_slice() const noexcept;

    private:

        storage_type m_data;
    };

    /*******************
     * slice_variant_t *
     *******************/

    template <class L>
    using slice_variant_t = xtl::mpl::cast_t<L, xtl::variant>;

    /***************
     * xaxis_range *
     ***************/
    
    template <class L>
    class xaxis_range
    {
    public:

        using value_type = slice_variant_t<L>;

        xaxis_range(const value_type& first, const value_type& last) noexcept;
        xaxis_range(value_type&& first, value_type&& last) noexcept;

        template <class A>
        using slice_type = xt::xrange<typename A::mapped_type>;

        template <class A>
        slice_type<A> build_islice(const A& axis) const;

    private:

        value_type m_first;
        value_type m_last;
    };

    /***********************
     * xaxis_stepped_range *
     ***********************/

    template <class L>
    class xaxis_stepped_range
    {
    public:

        using value_type = slice_variant_t<L>;
        using size_type = std::size_t;

        xaxis_stepped_range(const value_type& first, const value_type& last, size_type step) noexcept;
        xaxis_stepped_range(value_type&& first, value_type&& last, size_type step) noexcept;

        template <class A>
        using slice_type = xt::xstepped_range<typename A::mapped_type>;

        template <class A>
        slice_type<A> build_islice(const A& axis) const;

    private:

        value_type m_first;
        value_type m_last;
        size_type m_step;
    };

    /*************
     * xaxis_all *
     *************/

    class xaxis_all
    {
    public:

        template <class A>
        using slice_type = xt::xall<typename A::mapped_type>;

        template <class A>
        slice_type<A> build_islice(const A& axis) const;
    };

    /***************
     * xaxis_slice *
     ***************/

    template <class L = DEFAULT_LABEL_LIST>
    class xaxis_slice
    {
    public:

        using squeeze_type = slice_variant_t<L>;
        using storage_type = xtl::variant<xaxis_range<L>, xaxis_stepped_range<L>, xaxis_all, squeeze_type>;

        xaxis_slice() = default;
        template <class V>
        xaxis_slice(const V& slice);
        template <class V>
        xaxis_slice(V&& slice);

        template <class A>
        using slice_type = xt::xslice_variant<typename A::mapped_type>;

        template <class A>
        slice_type<A> build_islice(const A& axis) const;

        const squeeze_type* get_squeeze() const noexcept;

    private:

        storage_type m_data;
    };

    template <class L = DEFAULT_LABEL_LIST>
    xaxis_slice<L> range(const slice_variant_t<L>& first, const slice_variant_t<L>& last);

    template <class L = DEFAULT_LABEL_LIST>
    xaxis_slice<L> range(slice_variant_t<L>&& first, slice_variant_t<L>&& last);

    template <class S, class L = DEFAULT_LABEL_LIST>
    xaxis_slice<L> range(const slice_variant_t<L>& first, const slice_variant_t<L>& last, S step);

    template <class S, class L = DEFAULT_LABEL_LIST>
    xaxis_slice<L> range(slice_variant_t<L>&& first, slice_variant_t<L>&& last, S step);

    /****************************************
     * xaxis_extended_islice implementation *
     ****************************************/

    namespace detail
    {
        template <class S, class T>
        inline xaxis_extended_islice<S> convert_slice(typename xaxis_extended_islice<T>::all_type)
        {
            return xt::xall_tag();
        }

        template <class S, class T>
        inline xaxis_extended_islice<S> convert_slice(const typename xaxis_extended_islice<T>::squeeze_type& squeeze)
        {
            return static_cast<S>(squeeze);
        }

        template <class S, class T>
        inline xaxis_extended_islice<S> convert_slice(const xt::xslice_variant<T>& slice)
        {
            return xt::xslice_variant<S>(slice);
        }
    }

    template <class T>
    template <class S, typename>
    inline xaxis_extended_islice<T>::xaxis_extended_islice(S&& slice)
        : m_data(std::forward<S>(slice))
    {
    }

    template <class T>
    template <class S, typename>
    inline xaxis_extended_islice<T>::operator xaxis_extended_islice<S>() const noexcept
    {
        return xtl::visit([](const auto& arg) { return detail::convert_slice<S, T>(arg); }, m_data);
    }

    template <class T>
    inline auto xaxis_extended_islice<T>::get_all() const noexcept -> const all_type*
    {
        return xtl::get_if<all_type>(&m_data);
    }

    template <class T>
    inline auto xaxis_extended_islice<T>::get_squeeze() const noexcept -> const squeeze_type*
    {
        return xtl::get_if<squeeze_type>(&m_data);
    }

    template <class T>
    inline auto xaxis_extended_islice<T>::get_slice() const noexcept -> const slice_type*
    {
        return xtl::get_if<slice_type>(&m_data);
    }

    /******************************
     * xaxis_range implementation *
     ******************************/

    template <class V>
    inline xaxis_range<V>::xaxis_range(const value_type& first, const value_type& last) noexcept
        : m_first(first), m_last(last)
    {
    }

    template <class V>
    inline xaxis_range<V>::xaxis_range(value_type&& first, value_type&& last) noexcept
        : m_first(std::move(first)), m_last(std::move(last))
    {
    }

    template <class V>
    template <class A>
    inline auto xaxis_range<V>::build_islice(const A& axis) const -> slice_type<A>
    {
        return slice_type<A>(axis[m_first], axis[m_last] + 1);
    }

    /**************************************
     * xaxis_stepped_range implementation *
     **************************************/

    template <class V>
    inline xaxis_stepped_range<V>::xaxis_stepped_range(const value_type& first, const value_type& last, size_type step) noexcept
        : m_first(first), m_last(last), m_step(step)
    {
    }

    template <class V>
    inline xaxis_stepped_range<V>::xaxis_stepped_range(value_type&& first, value_type&& last, size_type step) noexcept
        : m_first(std::move(first)), m_last(std::move(last)), m_step(step)
    {
    }

    template <class V>
    template <class A>
    inline auto xaxis_stepped_range<V>::build_islice(const A& axis) const -> slice_type<A>
    {
        return slice_type<A>(axis[m_first], axis[m_last] + 1, m_step);
    }

    /****************************
     * xaxis_all implementation *
     ****************************/

    template <class A>
    inline auto xaxis_all::build_islice(const A& axis) const -> slice_type<A>
    {
        return slice_type<A>(axis.size());
    }

    /******************************
     * xaxis_slice implementation *
     ******************************/

    template <class L>
    template <class V>
    inline xaxis_slice<L>::xaxis_slice(const V& slice)
        : m_data(slice)
    {
    }

    template <class L>
    template <class V>
    inline xaxis_slice<L>::xaxis_slice(V&& slice)
        : m_data(std::move(slice))
    {
    }

    template <class L>
    template <class A>
    inline auto xaxis_slice<L>::build_islice(const A& axis) const -> slice_type<A>
    {
        return xtl::visit(
            xtl::make_overload([&axis](const auto& arg) { return slice_type<A>(arg.build_islice(axis)); },
                               [&axis](const squeeze_type&) -> slice_type<A> { throw std::runtime_error("build_islice forbidden for squeeze"); }),
            m_data);
    }

    template <class L>
    inline auto xaxis_slice<L>::get_squeeze() const noexcept -> const squeeze_type*
    {
        return xtl::get_if<squeeze_type>(&m_data);
    }

    /***********************************
     * helper functions implementation *
     ***********************************/

    template <class L>
    inline xaxis_slice<L> range(const slice_variant_t<L>& first, const slice_variant_t<L>& last)
    {
        return xaxis_slice<L>(xaxis_range<L>(first, last));
    }

    template <class L>
    inline xaxis_slice<L> range(slice_variant_t<L>&& first, slice_variant_t<L>&& last)
    {
        return xaxis_slice<L>(xaxis_range<L>(std::move(first), std::move(last)));
    }

    template <class S, class L>
    inline xaxis_slice<L> range(const slice_variant_t<L>& first, const slice_variant_t<L>& last, S step)
    {
        return xaxis_slice<L>(xaxis_stepped_range<L>(first, last, step));
    }

    template <class S, class L>
    inline xaxis_slice<L> range(slice_variant_t<L>&& first, slice_variant_t<L>&& last, S step)
    {
        return xaxis_slice<L>(xaxis_stepped_range<L>(std::move(first), std::move(last), step));
    }

}

#endif
