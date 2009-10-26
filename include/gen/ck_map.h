/*
    Copyright (c) 2007-2009 Takashi Kitao
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
    `  notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
    `  notice, this list of conditions and the following disclaimer in the
    `  documentation and/or other materials provided with the distribution.

    3. The name of the author may not be used to endorse or promote products
    `  derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*!
    @ingroup ckGen
    A sorted associative containers that contain unique key/value pairs.
    @tparam K The key type which is used to find the associated value.
    @tparam D The value type which is associated with the key type.
*/
template<class K, class D> class ckMap
{
public:
    ckDefineException(ExceptionInvalidArgument);
    ckDefineException(ExceptionNotFound);
    ckDefineException(ExceptionNotInitialized);
    ckDefineException(ExceptionSameKeyExists);

    /*!
        Constructs a ckMap.
    */
    ckMap()
    {
        m_hash_list = NULL;
        m_last_item1 = m_last_item2 = m_last_item3 = NULL;
        m_hash_size = 0;
        m_data_num = 0;
    }

    /*!
        Destructs this ckMap.
    */
    ~ckMap()
    {
        clear();

        if (m_hash_list)
        {
            ckDeleteArray(m_hash_list, ckList<MapItem>);
        }
    }

    /*!
        Initializes this ckMap whose hash list has the specified size.
        The size of the hash list must be more than zero.
        @param[in] hash_size The size of the hash list of this ckMap.
    */
    void init(u16 hash_size)
    {
        if (hash_size == 0)
        {
            ckThrow(ExceptionInvalidArgument);
        }

        clear();

        if (m_hash_list)
        {
            ckDeleteArray(m_hash_list, ckList<MapItem>);
        }

        m_hash_size = hash_size;
        ckNewArray(m_hash_list, ckList<MapItem>, m_hash_size);
    }

    /*!
        Returns the size of the hash list of this ckMap.
        @return The size of the hash list of this ckMap.
    */
    u16 getHashSize() const
    {
        return m_hash_size;
    }

    /*!
        Returns the number of the key/value pairs in this ckMap.
        @return The number of the key/value pairs in this ckMap.
    */
    u16 getDataNum() const
    {
        return m_data_num;
    }

    /*!
        Returns the value which is associated with the specified key.
        @param[in] key A key in this ckMap.
        @return The value which is associated with the specified key.
    */
    D* get(K key)
    {
        D* data = getN(key);

        if (!data)
        {
            ckThrow(ExceptionNotFound);
        }

        return data;
    }

    /*!
        Returns the value which is associated with the specified key.
        If the specified key isn't in this ckMap, returns NULL.
        @param[in] key A key in this ckMap.
        @return The value which is associated with the specified key.
    */
    D* getN(K key)
    {
        if (!m_hash_list)
        {
            ckThrow(ExceptionNotInitialized);
        }

        if (m_last_item1 && m_last_item1->key == key)
        {
            return &m_last_item1->data;
        }
        else if (m_last_item2 && m_last_item2->key == key)
        {
            MapItem* item = m_last_item1;
            m_last_item1 = m_last_item2;
            m_last_item2 = item;

            return &m_last_item1->data;
        }
        else if (m_last_item3 && m_last_item3->key == key)
        {
            MapItem* item = m_last_item1;
            m_last_item1 = m_last_item3;
            m_last_item3 = m_last_item2;
            m_last_item2 = item;

            return &m_last_item1->data;
        }
        else
        {
            s32 index = key % m_hash_size;
            ckList<MapItem>* hash_list = &m_hash_list[(index < 0) ? -index : index];

            for (typename ckList<MapItem>::Item* item = hash_list->getFirstN(); item; item = item->getNextN())
            {
                if (item->getSelf()->key == key)
                {
                    m_last_item3 = m_last_item2;
                    m_last_item2 = m_last_item1;
                    m_last_item1 = item->getSelf();

                    hash_list->addFirst(item);

                    return &m_last_item1->data;
                }
            }

            return NULL;
        }
    }

    /*!
        Adds a key/value pair to this ckMap.
        @param[in] key A key.
        @param[in] data A value.
    */
    void add(K key, const D& data)
    {
        if (!m_hash_list)
        {
            ckThrow(ExceptionNotInitialized);
        }

        if (getN(key))
        {
            ckThrow(ExceptionSameKeyExists);
        }

        MapItem* new_item = ckNew(MapItem);

        new_item->order_item.init(new_item);
        new_item->hash_item.init(new_item);
        new_item->key = key;
        new_item->data = data;

        m_order_list.addLast(&new_item->order_item);

        s32 index = key % m_hash_size;
        m_hash_list[(index < 0) ? -index : index].addLast(&new_item->hash_item);

        m_data_num++;
    }

    /*!
        Removes the specified key/value pair from this ckMap.
        @param[in] key A key in this ckMap.
    */
    void remove(K key)
    {
        if (!m_hash_list)
        {
            ckThrow(ExceptionNotInitialized);
        }

        if (m_last_item1 && m_last_item1->key == key)
        {
            ckDelete(m_last_item1, MapItem);
            m_data_num--;

            m_last_item1 = m_last_item2;
            m_last_item2 = m_last_item3;
            m_last_item3 = NULL;
        }
        else if (m_last_item2 && m_last_item2->key == key)
        {
            ckDelete(m_last_item2, MapItem);
            m_data_num--;

            m_last_item2 = m_last_item3;
            m_last_item3 = NULL;
        }
        else if (m_last_item3 && m_last_item3->key == key)
        {
            ckDelete(m_last_item3, MapItem);
            m_data_num--;

            m_last_item3 = NULL;
        }
        else
        {
            s32 index = key % m_hash_size;
            ckList<MapItem>* hash_list = &m_hash_list[(index < 0) ? -index : index];

            for (typename ckList<MapItem>::Item* item = hash_list->getFirstN(); item; item = item->getNextN())
            {
                if (item->getSelf()->key == key)
                {
                    ckDelete(item->getSelf(), MapItem);
                    m_data_num--;

                    return;
                }
            }

            ckThrow(ExceptionNotFound);
        }
    }

    /*!
        Removes all key/value pairs from this ckMap.
    */
    void clear()
    {
        m_order_list.clear();

        if (m_hash_list)
        {
            for (s32 i = 0; i < m_hash_size; i++)
            {
                ckList<MapItem>* hash_list = &m_hash_list[i];

                while (hash_list->hasItem())
                {
                    ckDelete(hash_list->getFirstN()->getSelf(), MapItem);
                }
            }

            m_last_item1 = m_last_item2 = m_last_item3 = NULL;
            m_data_num = 0;
        }
    }

    /*!
        Returns the first key in this ckMap. If the ckMap has no key/value pair, returns NULL.
        @return The first key in this ckMap.
    */
    const K* getFirstKeyN() const
    {
        if (!m_hash_list)
        {
            ckThrow(ExceptionNotInitialized);
        }

        return m_order_list.hasItem() ? &m_order_list.getFirstN()->getSelf()->key : NULL;
    }

    /*!
        Returns the last key in this ckMap. If the ckMap has no key/value pair, returns NULL.
        @return The last key in this ckMap.
    */
    const K* getLastKeyN() const
    {
        if (!m_hash_list)
        {
            ckThrow(ExceptionNotInitialized);
        }

        return m_order_list.hasItem() ? &m_order_list.getLastN()->getSelf()->key : NULL;
    }

    /*!
        Returns the previous key of the specified key.
        If the specified key has no previous key, returns NULL.
        @param[in] key A key in this ckMap.
        @return The previous key of the specified key.
    */
    const K* getPrevKeyN(K key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* prev = reinterpret_cast<MapItem*>(get(key))->order_item.getPrevN();

            return prev ? &prev->getSelf()->key : NULL;
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }

        return NULL;
    }

    /*!
        Returns the next key of the specified key.
        If the specified key has no next key, returns NULL.
        @param[in] key A key in this ckMap.
        @return The next key of the specified key.
    */
    const K* getNextKeyN(K key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* next = reinterpret_cast<MapItem*>(get(key))->order_item.getNextN();

            return next ? &next->getSelf()->key : NULL;
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }

        return NULL;
    }

    /*!
        Move the specified key/value pair to the first in this ckMap.
        @param[in] key A key in this ckMap.
    */
    void moveFirst(K key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* item = &reinterpret_cast<MapItem*>(get(key))->order_item;

            m_order_list.addFirst(item);
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }
    }

    /*!
        Move the specified key/value pair to the last in this ckMap.
        @param[in] key A key in this ckMap.
    */
    void moveLast(K key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* item = &reinterpret_cast<MapItem*>(get(key))->order_item;

            m_order_list.addLast(item);
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }
    }

    /*!
        Move the specified key/value pair before the another pair.
        @param[in] key A key in this ckMap.
        @param[in] next_key An another key in this ckMap.
    */
    void moveBefore(K key, K next_key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* item = &reinterpret_cast<MapItem*>(get(key))->order_item;
            typename ckList<MapItem>::Item* next = &reinterpret_cast<MapItem*>(get(next_key))->order_item;

            item->joinBefore(next);
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }
    }

    /*!
        Move the specified key/value pair after the another pair.
        @param[in] key A key in this ckMap.
        @param[in] prev_key An another key in this ckMap.
    */
    void moveAfter(K key, K prev_key)
    {
        ckTry
        {
            typename ckList<MapItem>::Item* item = &reinterpret_cast<MapItem*>(get(key))->order_item;
            typename ckList<MapItem>::Item* prev = &reinterpret_cast<MapItem*>(get(prev_key))->order_item;

            item->joinAfter(prev);
        }
        ckCatch(ExceptionNotFound)
        {
            ckThrow(ExceptionInvalidArgument);
        }
    }

private:
    struct MapItem
    {
        D data; // This data member must be at the top of this structure.
        typename ckList<MapItem>::Item order_item;
        typename ckList<MapItem>::Item hash_item;
        K key;
    };

    ckMap(const ckMap<K, D>&) {}
    void operator=(const ckMap<K, D>&) {}

    ckList<MapItem> m_order_list;
    ckList<MapItem>* m_hash_list;
    MapItem* m_last_item1;
    MapItem* m_last_item2;
    MapItem* m_last_item3;
    u16 m_hash_size;
    u16 m_data_num;
};
