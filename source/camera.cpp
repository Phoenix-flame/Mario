#include "camera.hpp"

Camera::Camera()
{
    this->pos = Point(0, 0);
    this->posBackground = Point(0, 0);
}

void Camera::move(int offset)
{
    if (this->pos.x + offset > 0)
    {
        canMove = false;
        return;
    }
    if (this->pos.x + offset < -4770)
    {
        canMove = false;
        return;
    }
    this->pos.x += offset;
    canMove = true;
}

void Camera::moveBackground(int offset)
{
    if (!canMove)
    {
        return;
    }
    this->posBackground.x += offset;
}

Point Camera::getPos()
{
    return pos;
}
