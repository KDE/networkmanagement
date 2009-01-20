#include "setting.h"

Setting::Setting(Setting::Type type)
    :m_type(type)
{

}

Setting::~Setting()
{

}

Setting::Type Setting::type() const
{
    return m_type;
}

// vim: sw=4 sts=4 et tw=100
