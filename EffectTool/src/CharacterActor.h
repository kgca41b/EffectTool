#pragma once
#include "DataTypes.h"
#include "Actor.h"
#include "CharacterData.h"
#include "Components.h"

namespace KGCA41B
{
	class CharacterActor : public Actor
	{
	public:
		virtual void OnInit(entt::registry& registry, AABB<3> collision_box) override;
		virtual void OnUpdate(entt::registry& registry) override;
		void SetCharacterData(entt::registry& registry, CharacterData data);
	};
}


