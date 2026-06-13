#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

class SelectProxy {
public:
	SelectProxy();
	~SelectProxy() = default;

	glm::vec4 GetColor() const;
private:
	uint32_t ID;
};