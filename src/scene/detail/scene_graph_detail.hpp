// 004-Scene: Internal node accessors for hierarchy/level. Not part of public contract.

#ifndef TENENGINE_SCENE_DETAIL_SCENE_GRAPH_DETAIL_HPP
#define TENENGINE_SCENE_DETAIL_SCENE_GRAPH_DETAIL_HPP

#include "scene/types.hpp"
#include <cstddef>

namespace te {
namespace scene {
namespace detail {

const char* GetNodeName(NodeId node);
void SetNodeName(NodeId node, char const* name);

void* GetNodeType(NodeId node);
void SetNodeType(NodeId node, void* typeTag);

bool GetNodeActive(NodeId node);
void SetNodeActive(NodeId node, bool active);

}  // namespace detail
}  // namespace scene
}  // namespace te

#endif
