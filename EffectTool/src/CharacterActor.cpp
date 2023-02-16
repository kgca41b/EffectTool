#include "CharacterActor.h"

void KGCA41B::CharacterActor::OnInit(entt::registry& registry, AABB<3> collision_box)
{
	collision_box_ = collision_box;

	entity_id_ = registry.create();
	//this->node_num_ = SpacePartition::GetInst()->UpdateNodeObjectBelongs(0, collision_box_, entity_id_);

	entt::type_hash<Transform> type_hash_transform;
	Transform transform;
	transform.local = XMMatrixIdentity();
	transform.world = XMMatrixIdentity();
	registry.emplace<Transform>(entity_id_, transform);

	transform_tree_.root_node = make_shared<TransformTreeNode>(type_hash_transform.value());

	transform_tree_.root_node->OnUpdate(registry, entity_id_);
}

void KGCA41B::CharacterActor::OnUpdate(entt::registry& registry)
{
	//this->node_num_ = SpacePartition::GetInst()->UpdateNodeObjectBelongs(0, collision_box_, entity_id_);
	//vector<int> node_to_search = SpacePartition::GetInst()->FindCollisionSearchNode(0, collision_box_);
	transform_tree_.root_node->OnUpdate(registry, entity_id_);
}

void KGCA41B::CharacterActor::SetCharacterData(entt::registry& registry, CharacterData data)
{
	KGCA41B::Animation animation;
	animation.anim_id = data.anim_id;
	registry.emplace_or_replace<KGCA41B::Animation>(entity_id_, animation);

	KGCA41B::SkeletalMesh skm;
	skm.local = XMMatrixIdentity();
	skm.world = XMMatrixIdentity();
	skm.mesh_id = data.skm_id;
	skm.shader_id = data.vs_id;
	animation.anim_id = data.anim_id;
	registry.emplace_or_replace<KGCA41B::SkeletalMesh>(entity_id_, skm);

	KGCA41B::Skeleton skeleton;
	skeleton.skeleton_id = data.skeleton_id;
	skeleton.local = XMMatrixIdentity();
	skeleton.world = XMMatrixIdentity();
	KGCA41B::Material material;
	material.shader_id = data.ps_id;
	material.texture_id.push_back(data.texture_id);
	registry.emplace_or_replace<KGCA41B::Skeleton>(entity_id_, skeleton);
}
