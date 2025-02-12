#include "ControllerMgr.h"

MAA_CTRL_NS_BEGIN

ControllerMgr::ControllerMgr(MaaControllerCallback callback, void* callback_arg)
    : callback_(callback), callback_arg_(callback_arg)
{}

ControllerMgr::~ControllerMgr() {}

bool ControllerMgr::set_option(ControllerOptionKey key, const std::string& value)
{
    return false;
}

bool ControllerMgr::connecting() const
{
    return false;
}

bool ControllerMgr::connected() const
{
    return false;
}

std::vector<unsigned char> ControllerMgr::get_image() const
{
    return std::vector<unsigned char>();
}

std::string ControllerMgr::get_uuid() const
{
    return std::string();
}

MAA_CTRL_NS_END
