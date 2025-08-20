/**
 * @file Utility.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */




 #include <string>
 #include <sys/_stdint.h>
 
 #include "Common/Status.h"
 #include "Common/Assert.hpp"
 #include "Common/Logger/Logger.h"
 #include "Common/Time/TimeUtils.h"
 #include "JARVIS/Include/TypeDefinitions.h"
 #include "Utility.h"
 
 
 namespace muffin { namespace im {
 
     bool IsBitArea(const jvs::node_area_e area)
     {
         switch (area)
         {
         case jvs::node_area_e::COILS:
         case jvs::node_area_e::DISCRETE_INPUT:
         case jvs::node_area_e::SM:
         case jvs::node_area_e::X:
         case jvs::node_area_e::Y:
         case jvs::node_area_e::M:
         case jvs::node_area_e::L:
         case jvs::node_area_e::F:
         case jvs::node_area_e::V:
         case jvs::node_area_e::B:
         case jvs::node_area_e::TS:
         case jvs::node_area_e::TC:
         case jvs::node_area_e::LTS:
         case jvs::node_area_e::LTC:
         case jvs::node_area_e::STS:
         case jvs::node_area_e::STC:
         case jvs::node_area_e::LSTS:
         case jvs::node_area_e::LSTC:
         case jvs::node_area_e::CS:
         case jvs::node_area_e::CC:
         case jvs::node_area_e::LCS:
         case jvs::node_area_e::LCC:
         case jvs::node_area_e::SB:
         case jvs::node_area_e::S:
         case jvs::node_area_e::DX:
         case jvs::node_area_e::DY:
             return true;
         case jvs::node_area_e::INPUT_REGISTER:
         case jvs::node_area_e::HOLDING_REGISTER:
         case jvs::node_area_e::SD:
         case jvs::node_area_e::D:
         case jvs::node_area_e::W:
         case jvs::node_area_e::TN:
         case jvs::node_area_e::CN:
         case jvs::node_area_e::SW:
         case jvs::node_area_e::Z:
             return false;
         /**
          * @todo DoubleWord 어떻게 처리할 것인지 확인 필요 @김주성 
          * 
          */
         case jvs::node_area_e::LTN:
         case jvs::node_area_e::STN:
         case jvs::node_area_e::LSTN:
         case jvs::node_area_e::LCN:
         case jvs::node_area_e::LZ:
             LOG_WARNING(logger,"Double Word");
             return false;
         default:
             LOG_ERROR(logger,"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
             ASSERT((true),"UNDEFINED NODE AREA %d",static_cast<uint16_t>(area));
             return false;
         }
     }
 }}