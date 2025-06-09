#pragma once

// Internal
#include <hellengine/ecs/shared.h>

namespace hellengine
{

	namespace ecs
	{

        struct Transform
        {
            /*glm::vec3 localPosition{ 0.0f };
            glm::quat localRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
            glm::vec3 localScale{ 1.0f };

            glm::mat4 worldMatrix{ 1.0f };

            Transform* parent = nullptr;
            std::vector<Transform*> children;

        private:
            bool dirty = true;

        public:
            void SetPosition(const glm::vec3& pos) { localPosition = pos; MarkDirty(); }
            void SetRotation(const glm::quat& rot) { localRotation = rot; MarkDirty(); }
            void SetScale(const glm::vec3& scale) { localScale = scale; MarkDirty(); }

            void SetParent(Transform* newParent) {
                if (parent == newParent) return;

                if (parent) {
                    auto& siblings = parent->children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
                }

                parent = newParent;

                if (parent) {
                    parent->children.push_back(this);
                }

                MarkDirty();
            }

            glm::vec3 GetLocalPosition() const { return localPosition; }
            glm::quat GetLocalRotation() const { return localRotation; }
            glm::vec3 GetLocalScale()   const { return localScale; }

            glm::mat4 GetWorldMatrix() {
                UpdateWorldMatrix();
                return worldMatrix;
            }

            glm::vec3 GetWorldPosition() {
                UpdateWorldMatrix();
                return glm::vec3(worldMatrix[3]);
            }

            glm::quat GetWorldRotation() {
                UpdateWorldMatrix();
                glm::vec3 skew, scale, translation;
                glm::vec4 perspective;
                glm::quat rotation;
                glm::decompose(worldMatrix, scale, rotation, translation, skew, perspective);
                return rotation;
            }

            glm::vec3 GetWorldScale() {
                UpdateWorldMatrix();
                glm::vec3 skew, scale, translation;
                glm::vec4 perspective;
                glm::quat rotation;
                glm::decompose(worldMatrix, scale, rotation, translation, skew, perspective);
                return scale;
            }

        private:
            void MarkDirty() {
                if (dirty) return;
                dirty = true;
                for (Transform* child : children)
                    child->MarkDirty();
            }

            void UpdateWorldMatrix() {
                if (!dirty) return;

                glm::mat4 local =
                    glm::translate(glm::mat4(1.0f), localPosition) *
                    glm::mat4(localRotation) *
                    glm::scale(glm::mat4(1.0f), localScale);

                if (parent) {
                    worldMatrix = parent->GetWorldMatrix() * local;
                }
                else {
                    worldMatrix = local;
                }

                dirty = false;
            }*/
        };

	} // namespace ecs

} // namespace hellengine