/**
 * @file Node.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#include "Common/Logger/Logger.h"
#include "Node.h"


namespace muffin { namespace jarvis { namespace config {

    Node::Node(const std::string& key)
        : Base(key)
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    Node::~Node()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    Node& Node::operator=(const Node& obj)
    {
        if (this != &obj)
        {
            mMap       =  obj.mMap;
            mOrd       =  obj.mOrd;
            mDt        =  obj.mDt;
            mStrAddr   =  obj.mStrAddr;
            mUintAddr  =  obj.mUintAddr;
            mBit       =  obj.mBit;
            mArea      =  obj.mArea;
            mQty       =  obj.mQty;
            mScl       =  obj.mScl;
            mOffset    =  obj.mOffset;
            mNodeId    =  obj.mNodeId;
            mFmt       =  obj.mFmt;
            mName      =  obj.mName;
            mUnit      =  obj.mUnit;
            mUID       =  obj.mUID;
            mPID       =  obj.mPID;
        }
        
        return *this;
    }

    bool Node::operator==(const Node& obj) const
    {
       return (
            mMap        == obj.mMap         &&    
            mOrd        == obj.mOrd         &&    
            mDt         == obj.mDt          &&
            mStrAddr    == obj.mStrAddr     &&    
            mUintAddr   == obj.mUintAddr    &&    
            mBit        == obj.mBit         &&
            mArea       == obj.mArea        &&
            mQty        == obj.mQty         &&
            mScl        == obj.mScl         &&
            mOffset     == obj.mOffset      &&
            mNodeId     == obj.mNodeId      &&
            mFmt        == obj.mFmt         &&
            mName       == obj.mName        &&
            mUnit       == obj.mUnit        &&
            mUID        == obj.mUID         &&
            mPID        == obj.mPID         
        );
    }

    bool Node::operator!=(const Node& obj) const
    {
        return !(*this == obj);
    }

    Status Node::SetNodeID(const std::string& nodeid)
    {
        mNodeId = nodeid;
        if (mNodeId == nodeid)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetAddressType(const uint8_t& adtp)
    {
        mAddrType = adtp;
        if (mAddrType == adtp)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetAddrress(const uint16_t& addr)
    {
        mUintAddr = addr;
        if (mUintAddr == addr)
        {
            mIsAddrSet = true;
            mIsAddrStr = false;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetModbusArea(const uint8_t& area)
    {
        mArea = area;
        if (mArea == area)
        {
            mIsAreaSet = true;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetBitIndex(const uint8_t& bit)
    {
        mBit = bit;
        if (mBit == bit)
        {
            mIsBitSet = true;
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetModbusRegisterQuantity(const uint8_t& qty)
    {
        mQty = qty;
        if (mQty == qty)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataScale(const int8_t& scl)
    {
        mScl = scl;
        if (mScl == scl)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataOffset(const float& ofst)
    {
        mOffset = ofst;
        if (mOffset == ofst)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataOrder(const std::vector<std::string>& ord)
    {
        mOrd = ord;
        if (mOrd == ord)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataTypes(const std::vector<uint8_t>& dt)
    {
        mDt = dt;
        if (mDt == dt)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetFormatString(const std::string& fmt)
    {
        mFmt = fmt;
        if (mFmt == fmt)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetMappingRules(const std::map<std::uint16_t, std::string>& map)
    {
        mMap = map;
        if (mMap == map)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataName(const std::string& name)
    {
        mName = name;
        if (mName == name)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetDataUnit(const std::string& unit)
    {
        mUnit = unit;
        if (mUnit == unit)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetUID(const std::string& uid)
    {
        mUID = uid;
        if (mUID == uid)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status Node::SetPID(const std::string& pid)
    {
        mPID = pid;
        if (mPID == pid)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    const std::string& Node::GetNodeID() const
    {
        return mNodeId;
    }

    const uint8_t& Node::GetAddressType() const
    {
        return mAddrType;
    }

    const uint16_t& Node::GetUint16Addrress() const
    {
        return mUintAddr;
    }

    const uint8_t& Node::GetModbusArea() const
    {
        return mArea;
    }

    const uint8_t& Node::GetBitIndex() const
    {
        return mBit;
    }

    const uint8_t& Node::GetModbusRegisterQuantity() const
    {
        return mQty;
    }

    const int8_t& Node::GetDataScale() const
    {
        return mScl;
    }

    const float& Node::GetDataOffset() const
    {
        return mOffset;
    }

    const std::vector<std::string>& Node::GetDataOrder() const
    {
        return mOrd;
    }

    const std::vector<uint8_t>& Node::GetDataTypes() const
    {
        return mDt;
    }

    const std::string& Node::GetFormatString() const
    {
        return mFmt;
    }

    const std::map<std::uint16_t, std::string>& Node::GetMappingRules() const
    {
        return mMap;
    }

    const std::string& Node::GetDataName() const
    {
        return mName;
    }

    const std::string& Node::GetDataUnit() const
    {
        return mUnit;
    }

    const std::string& Node::GetUID() const
    {
        return mUID;
    }

    const std::string& Node::GetPID() const
    {
        return mPID;
    }

}}}


