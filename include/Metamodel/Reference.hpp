/**
 * @file Reference.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Reference to either a model element of the same or another AAS or to an external
 * entity. A reference is an ordered list of keys.
 * 
 * @details
 * A model reference is an ordered list of keys, each key referencing an element. The complete list 
 * of keys may for example be concatenated to a path that then gives unique access to an element. 
 * A global reference is a reference to an external entity.
 * 
 * The cardinality of member variables is as follows:
 *  - @var type: 1
 *  - @var referredSemanticId: 0..1
 *  - @var key: 1..*
 * 
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <vector>

#include "./Key.hpp"
#include "./TypeDefinitions.hpp"



namespace muffin { namespace aas {


    class Reference
    {
    private:
        reference_types_e type;
        std::weak_ptr<Reference> referredSemanticId;
        std::vector<Key> key;
    };
}}