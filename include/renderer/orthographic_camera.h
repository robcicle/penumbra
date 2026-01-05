#pragma once

namespace penumbra 
{
	class COrthographicCamera
	{
	public:
		COrthographicCamera(float flLeft, float flRight, float flBottom, float flTop);
		~COrthographicCamera() = default;

		void SetProjection(float flLeft, float flRight, float flBottom, float flTop);

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		float GetRotation() const { return m_flRotation; }
		void SetRotation(float rotation) { m_flRotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_matProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_matViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_matViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_matProjectionMatrix;
		glm::mat4 m_matViewMatrix;
		glm::mat4 m_matViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_flRotation = 0.0f;
	};

}