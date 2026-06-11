#include "SelectProxy.hpp"

static uint32_t nextID = -1;

SelectProxy::SelectProxy() {
	this->ID = nextID++;
}

glm::vec4 SelectProxy::GetColor() const {
    return glm::vec4(
        (ID >> 0 & 0xFF) / 255.f, 
        (ID >> 8 & 0xFF) / 255.f, 
        (ID >> 16 & 0xFF) / 255.f,
        1.0f);
}