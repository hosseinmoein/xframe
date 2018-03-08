/***************************************************************************
* Copyright (c) 2017, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XFRAME_XCOORDINATE_HPP
#define XFRAME_XCOORDINATE_HPP

#include <map>

#include "xtl/xiterator_base.hpp"
#include "xaxis_variant.hpp"
#include "xframe_config.hpp"

namespace xf
{
    namespace join
    {
        enum class join_id
        {
            outer_id,
            inner_id
        };

        struct outer
        {
            static constexpr join_id id() { return join_id::outer_id; }
        };
        
        struct inner
        {
            static constexpr join_id id() { return join_id::inner_id; }
        };
    }

    class xfull_coordinate {};

    template <class K, class S, class MT = hash_map_tag, class L = DEFAULT_LABEL_LIST>
    class xcoordinate
    {
    public:

        using self_type = xcoordinate<K, S, MT, L>;
        using label_list = L;
        using axis_type = xaxis_variant<L, S, MT>;
        using map_type = std::map<K, axis_type>;
        using key_type = typename map_type::key_type;
        using mapped_type = typename map_type::mapped_type;
        using index_type = S;
        using value_type = typename map_type::value_type;
        using reference = typename map_type::reference;
        using const_reference = typename map_type::const_reference;
        using pointer = typename map_type::pointer;
        using const_pointer = typename map_type::const_pointer;
        using size_type = typename map_type::size_type;
        using difference_type = typename map_type::difference_type;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;
        using key_iterator = xtl::xkey_iterator<map_type>;

        explicit xcoordinate(const map_type& axes);
        explicit xcoordinate(map_type&& axes);
        xcoordinate(std::initializer_list<value_type> init);
        template <class... LB>
        explicit xcoordinate(std::pair<K, xaxis<LB, S, MT>>... axes);

        bool empty() const;
        size_type size() const;
        void clear();
        
        bool contains(const key_type& key) const;
        const mapped_type& operator[](const key_type& key) const;

        template <class KB, class LB>
        index_type operator[](const std::pair<KB, LB>& key) const;

        const map_type& data() const noexcept;

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        key_iterator key_begin() const noexcept;
        key_iterator key_end() const noexcept;

        template <class Join, class... Args>
        std::pair<bool, bool> broadcast(const Args&... coordinates);

        bool operator==(const self_type& rhs) const noexcept;
        bool operator!=(const self_type& rhs) const noexcept;

    private:
        
        template <class LB1, class... LB>
        void insert_impl(std::pair<K, xaxis<LB1, S, MT>> axis, std::pair<K, xaxis<LB, S, MT>>... axes);
        void insert_impl();

        template <class Join, class... Args>
        std::pair<bool, bool> broadcast_impl(const self_type& c, const Args&... coordinates);
        template <class Join, class... Args>
        std::pair<bool, bool> broadcast_impl(const xfull_coordinate& c, const Args&... coordinates);
        template <class Join>
        std::pair<bool, bool> broadcast_impl();

        template <class Join, class... Args>
        std::pair<bool, bool> broadcast_empty(const self_type& c, const Args&... coordinates);
        template <class Join, class... Args>
        std::pair<bool, bool> broadcast_empty(const xfull_coordinate& c, const Args&... coordinates);
        template <class Join>
        std::pair<bool, bool> broadcast_empty();

        map_type m_coordinate;
    };

    template <class OS, class K, class S, class MT, class L>
    OS& operator<<(OS& out, const xcoordinate<K, S, MT, L>& c);

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L> coordinate(const std::map<K, xaxis_variant<L, S, MT>>& axes);

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L> coordinate(std::map<K, xaxis_variant<L, S, MT>>&& axes);

    template <class K, class S, class MT, class... L>
    xcoordinate<K, S, MT> coordinate(std::pair<K, xaxis<L, S, MT>>... axes);

    template <class Join, class K, class S, class MT, class L, class... Args>
    std::pair<bool, bool> broadcast_coordinates(xcoordinate<K, S, MT, L>& output, const Args&... coordinates);

    /****************************
     * coordinate metafunctions *
     ****************************/

    namespace detail
    {
        template <class T>
        struct is_coordinate_impl : std::false_type
        {
        };

        template <class K, class S, class MT, class L>
        struct is_coordinate_impl<xcoordinate<K, S, MT, L>> : std::true_type
        {
        };

        template <class T>
        struct is_coordinate_map_impl : std::false_type
        {
        };

        template <class K, class L, class S, class MT>
        struct is_coordinate_map_impl<std::map<K, xaxis_variant<L, S, MT>>>
            : std::true_type
        {
        };

        template <class T>
        struct get_coordinate_type_impl;

        template <class K, class L, class S, class MT>
        struct get_coordinate_type_impl<std::map<K, xaxis_variant<L, S, MT>>>
        {
            using type = xcoordinate<K, S, MT, L>;
        };
    }

    template <class T>
    struct is_coordinate : detail::is_coordinate_impl<std::decay_t<T>>
    {
    };

    template <class T>
    struct is_coordinate_map : detail::is_coordinate_map_impl<std::decay_t<T>>
    {
    };

    template <class T>
    struct enable_coordinate_map
        : std::enable_if<is_coordinate_map<T>::value, xt::void_t<>>
    {
    };

    template <class T>
    using enable_coordinate_map_t = typename enable_coordinate_map<T>::type;

    template <class T>
    struct get_coordinate_type : detail::get_coordinate_type_impl<std::decay_t<T>>
    {
    };

    template <class T>
    using get_coordinate_type_t = typename get_coordinate_type<T>::type;

    /******************************
     * xcoordinate implementation *
     ******************************/

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L>::xcoordinate(const map_type& axes)
        : m_coordinate(axes)
    {
    }

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L>::xcoordinate(map_type&& axes)
        : m_coordinate(std::move(axes))
    {
    }

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L>::xcoordinate(std::initializer_list<value_type> init)
        : m_coordinate(init)
    {
    }

    template <class K, class S, class MT, class L>
    template <class... LB>
    inline xcoordinate<K, S, MT, L>::xcoordinate(std::pair<K, xaxis<LB, S, MT>>... axes)
    {
        insert_impl(std::move(axes)...);
    }
    
    template <class K, class S, class MT, class L>
    inline bool xcoordinate<K, S, MT, L>::empty() const
    {
        return m_coordinate.empty();
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::size() const -> size_type
    {
        return m_coordinate.size();
    }

    template <class K, class S, class MT, class L>
    inline void xcoordinate<K, S, MT, L>::clear()
    {
        m_coordinate.clear();
    }

    template <class K, class S, class MT, class L>
    inline bool xcoordinate<K, S, MT, L>::contains(const key_type& key) const
    {
        return m_coordinate.find(key) != m_coordinate.end();
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::operator[](const key_type& key) const -> const mapped_type&
    {
        return m_coordinate.at(key);
    }

    template <class K, class S, class MT, class L>
    template <class KB, class LB>
    inline auto xcoordinate<K, S, MT, L>::operator[](const std::pair<KB, LB>& key) const -> index_type
    {
        return (*this)[key.first][key.second];
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::data() const noexcept -> const map_type&
    {
        return m_coordinate;
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::begin() const noexcept -> const_iterator
    {
        return cbegin();
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::end() const noexcept -> const_iterator
    {
        return cend();
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::cbegin() const noexcept -> const_iterator
    {
        return m_coordinate.cbegin();
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::cend() const noexcept -> const_iterator
    {
        return m_coordinate.cend();
    }
    
    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::key_begin() const noexcept -> key_iterator
    {
        return key_iterator(begin());
    }

    template <class K, class S, class MT, class L>
    inline auto xcoordinate<K, S, MT, L>::key_end() const noexcept -> key_iterator
    {
        return key_iterator(end());
    }

    template <class K, class S, class MT, class L>
    template <class Join, class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast(const Args&... coordinates)
    {
        return empty() ? broadcast_empty<Join>(coordinates...) : broadcast_impl<Join>(coordinates...);
    }

    template <class K, class S, class MT, class L>
    inline bool xcoordinate<K, S, MT, L>::operator==(const self_type& rhs) const noexcept
    {
        return m_coordinate == rhs.m_coordinate;
    }

    template <class K, class S, class MT, class L>
    inline bool xcoordinate<K, S, MT, L>::operator!=(const self_type& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <class K, class S, class MT, class L>
    template <class LB1, class... LB>
    inline void xcoordinate<K, S, MT, L>::insert_impl(std::pair<K, xaxis<LB1, S, MT>> axis, std::pair<K, xaxis<LB, S, MT>>... axes)
    {
        m_coordinate.insert(value_type(std::move(axis.first), std::move(axis.second)));
        insert_impl(std::move(axes)...);
    }

    template <class K, class S, class MT, class L>
    inline void xcoordinate<K, S, MT, L>::insert_impl()
    {
    }

    namespace detail
    {
        template <class Join>
        struct axis_broadcast;

        template <>
        struct axis_broadcast<join::outer>
        {
            template <class A>
            static bool apply(A& output, const A& input)
            {
                return output.merge(input);
            }
        };

        template <>
        struct axis_broadcast<join::inner>
        {
            template <class A>
            static bool apply(A& output, const A& input)
            {
                return output.intersect(input);
            }
        };
    }

    template <class K, class S, class MT, class L>
    template <class Join, class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_impl(const self_type& c, const Args&... coordinates)
    {
        auto res = broadcast_impl<Join>(coordinates...);
        for(auto iter = c.begin(); iter != c.end(); ++iter)
        {
            auto inserted = m_coordinate.insert(*iter);
            if(inserted.second)
            {
                res.first = false;
            }
            else
            {
                const auto& key = inserted.first->first;
                auto& axis = inserted.first->second;
                res.second &= detail::axis_broadcast<Join>::apply(axis, c[key]);
            } 
        }
        return res;
    }

    template <class K, class S, class MT, class L>
    template <class Join, class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_impl(const xfull_coordinate& /*c*/, const Args&... coordinates)
    {
        return broadcast_impl<Join>(coordinates...);
    }

    template <class K, class S, class MT, class L>
    template <class Join>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_impl()
    {
        return { true, true };
    }

    template <class K, class S, class MT, class L>
    template <class Join, class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_empty(const self_type& c, const Args&... coordinates)
    {
        m_coordinate = c.m_coordinate;
        return broadcast_impl<Join>(coordinates...);
    }

    template <class K, class S, class MT, class L>
    template <class Join, class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_empty(const xfull_coordinate& /*c*/, const Args&... coordinates)
    {
        return broadcast_empty<Join>(coordinates...);
    }

    template <class K, class S, class MT, class L>
    template <class Join>
    inline std::pair<bool, bool> xcoordinate<K, S, MT, L>::broadcast_empty()
    {
        return broadcast_impl<Join>();
    }

    template <class OS, class K, class S, class MT, class L>
    inline OS& operator<<(OS& out, const xcoordinate<K, S, MT, L>& c)
    {
        for(auto& v: c)
        {
            out << v.first << ": " << v.second << std::endl;
        }
        return out;
    }

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L> coordinate(const std::map<K, xaxis_variant<L, S, MT>>& axes)
    {
        return xcoordinate<K, S, MT, L>(axes);
    }

    template <class K, class S, class MT, class L>
    xcoordinate<K, S, MT, L> coordinate(std::map<K, xaxis_variant<L, S, MT>>&& axes)
    {
        return xcoordinate<K, S, MT, L>(std::move(axes));
    }

    template <class K, class S, class MT, class... L>
    inline xcoordinate<K, S, MT> coordinate(std::pair<K, xaxis<L, S, MT>>... axes)
    {
        return xcoordinate<K, S, MT>(std::move(axes)...);
    }

    template <class Join, class K, class S, class MT, class L, class... Args>
    std::pair<bool, bool> broadcast_coordinates(xcoordinate<K, S, MT, L>& output, const Args&... coordinates)
    {
        return output.template broadcast<Join>(coordinates...);
    }
}

#endif

