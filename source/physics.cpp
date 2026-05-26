#include "physics.hpp"
#include "world.hpp"
#include "player.hpp"
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace
{
    const float PIXELS_PER_METER = 32.0f;
    const int PLATFORM_PROBE_MARGIN = 16;
    const int CONTACT_SKIN = 2;

    enum ContactSide
    {
        CONTACT_NONE,
        CONTACT_LEFT,
        CONTACT_RIGHT,
        CONTACT_TOP,
        CONTACT_BOTTOM
    };

    bool isEnemy(Object *obj)
    {
        return obj->getType() == GOOMBA || obj->getType() == KOOPA;
    }

    bool isPlayerEnemyPair(Object *a, Object *b)
    {
        return (a->getType() == PLAYER && isEnemy(b)) ||
               (b->getType() == PLAYER && isEnemy(a));
    }

    bool shouldIgnoreEnemyCollision(Object *moving_obj, Object *solid_obj)
    {
        if (!isPlayerEnemyPair(moving_obj, solid_obj))
        {
            return false;
        }

        Player *player = (moving_obj->getType() == PLAYER) ? (Player *)moving_obj : (Player *)solid_obj;
        return player->isInvincible();
    }

    bool supportsTopCollision(Object *obj)
    {
        return obj->getType() == PLAYER || obj->getType() == G_BULLET;
    }

    bool shouldApplyTopCollision(Object *obj)
    {
        return obj->getType() == G_BULLET || ((Player *)obj)->getState() == JUMP;
    }

    bool horizontalOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.x < b.x + b.w && a.x + a.w > b.x;
    }

    bool verticalOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.y < b.y + b.h && a.y + a.h > b.y;
    }

    bool expandedBoundsOverlap(const Rectangle &moving, const Rectangle &bounds)
    {
        Rectangle expanded(Point(bounds.x - PLATFORM_PROBE_MARGIN, bounds.y - PLATFORM_PROBE_MARGIN),
                           Point(bounds.x + bounds.w + PLATFORM_PROBE_MARGIN, bounds.y + bounds.h + PLATFORM_PROBE_MARGIN));
        return moving.x < expanded.x + expanded.w && moving.x + moving.w > expanded.x &&
               moving.y < expanded.y + expanded.h && moving.y + moving.h > expanded.y;
    }

    Rectangle objectRect(Object *obj)
    {
        return Rectangle(obj->getPos(), obj->getPos() + obj->getSize());
    }

    Rectangle inflateRect(const Rectangle &rect, int skin)
    {
        return Rectangle(Point(rect.x - skin, rect.y - skin),
                         Point(rect.x + rect.w + skin, rect.y + rect.h + skin));
    }

    b2Vec2 toWorldCenter(const Rectangle &rect)
    {
        return b2Vec2((rect.x + rect.w * 0.5f) / PIXELS_PER_METER,
                      (rect.y + rect.h * 0.5f) / PIXELS_PER_METER);
    }

    Object *bodyObject(b2Body *body)
    {
        return reinterpret_cast<Object *>(body->GetUserData().pointer);
    }

    void attachFixture(b2World &world, Object *object, const Rectangle &rect, b2BodyType bodyType)
    {
        b2BodyDef body_def;
        body_def.type = bodyType;
        body_def.position = toWorldCenter(rect);
        body_def.userData.pointer = reinterpret_cast<uintptr_t>(object);

        if (bodyType == b2_dynamicBody)
        {
            body_def.gravityScale = 0.0f;
            body_def.fixedRotation = true;
            body_def.awake = true;
        }

        b2Body *body = world.CreateBody(&body_def);

        b2PolygonShape shape;
        shape.SetAsBox((rect.w * 0.5f) / PIXELS_PER_METER,
                       (rect.h * 0.5f) / PIXELS_PER_METER);

        b2FixtureDef fixture_def;
        fixture_def.shape = &shape;
        fixture_def.isSensor = true;
        if (bodyType == b2_dynamicBody)
        {
            fixture_def.density = 1.0f;
        }
        body->CreateFixture(&fixture_def);
    }

    bool closeEnough(int gap)
    {
        return gap >= -CONTACT_SKIN && gap <= CONTACT_SKIN;
    }

    ContactSide sideFromRects(const Rectangle &moving, const Rectangle &target)
    {
        const int gap_to_floor = target.y - (moving.y + moving.h);
        const int gap_to_ceiling = moving.y - (target.y + target.h);
        const int gap_to_right_wall = target.x - (moving.x + moving.w);
        const int gap_to_left_wall = moving.x - (target.x + target.w);

        if (horizontalOverlap(moving, target) && closeEnough(gap_to_floor))
        {
            return CONTACT_BOTTOM;
        }
        if (horizontalOverlap(moving, target) && closeEnough(gap_to_ceiling))
        {
            return CONTACT_TOP;
        }
        if (verticalOverlap(moving, target) && closeEnough(gap_to_right_wall))
        {
            return CONTACT_LEFT;
        }
        if (verticalOverlap(moving, target) && closeEnough(gap_to_left_wall))
        {
            return CONTACT_RIGHT;
        }

        int moving_center_x = moving.x + moving.w / 2;
        int moving_center_y = moving.y + moving.h / 2;
        int target_center_x = target.x + target.w / 2;
        int target_center_y = target.y + target.h / 2;

        int dx = moving_center_x - target_center_x;
        int dy = moving_center_y - target_center_y;
        int combined_half_w = (moving.w + target.w) / 2;
        int combined_half_h = (moving.h + target.h) / 2;
        int overlap_x = combined_half_w - std::abs(dx);
        int overlap_y = combined_half_h - std::abs(dy);

        if (overlap_x <= 0 || overlap_y <= 0)
        {
            return CONTACT_NONE;
        }

        if (overlap_x < overlap_y)
        {
            return dx < 0 ? CONTACT_LEFT : CONTACT_RIGHT;
        }

        return dy < 0 ? CONTACT_BOTTOM : CONTACT_TOP;
    }

    void notifyContact(Object *moving, Object *target, ContactSide side,
                       const std::function<void(Object *)> &hitEnemiesAbove)
    {
        switch (side)
        {
        case CONTACT_LEFT:
            moving->notifyCollisionRight(target);
            target->notifyCollisionLeft(moving);
            break;
        case CONTACT_RIGHT:
            moving->notifyCollisionLeft(target);
            target->notifyCollisionRight(moving);
            break;
        case CONTACT_TOP:
            if (supportsTopCollision(moving) && shouldApplyTopCollision(moving))
            {
                moving->notifyCollisionTop(target);
                if (moving->getType() == PLAYER)
                {
                    hitEnemiesAbove(target);
                }
            }
            break;
        case CONTACT_BOTTOM:
            moving->notifyCollisionBottom(target);
            break;
        default:
            break;
        }
    }

    void notifyDistances(Object *moving, const Rectangle &moving_rect, const Rectangle &target_rect)
    {
        if (horizontalOverlap(moving_rect, target_rect))
        {
            if (moving_rect.y + moving_rect.h <= target_rect.y)
            {
                moving->notifyDistToPlatform(target_rect.y - (moving_rect.y + moving_rect.h));
            }
            else if (supportsTopCollision(moving) && moving_rect.y >= target_rect.y + target_rect.h)
            {
                moving->notifyDistToCeil(moving_rect.y - (target_rect.y + target_rect.h));
            }
        }
    }

    class CollisionContactListener : public b2ContactListener
    {
    public:
        CollisionContactListener(Object *_moving, const std::function<void(Object *)> &_hitEnemiesAbove) :
            moving(_moving),
            hitEnemiesAbove(_hitEnemiesAbove),
            onFloor(false) {}

        void BeginContact(b2Contact *contact) override
        {
            Object *a = bodyObject(contact->GetFixtureA()->GetBody());
            Object *b = bodyObject(contact->GetFixtureB()->GetBody());

            Object *target = nullptr;
            if (a == moving)
            {
                target = b;
            }
            else if (b == moving)
            {
                target = a;
            }
            else
            {
                return;
            }

            if (target == nullptr || target == moving || target->dead || shouldIgnoreEnemyCollision(moving, target))
            {
                return;
            }

            Rectangle moving_rect = objectRect(moving);
            Rectangle target_rect = objectRect(target);
            ContactSide side = sideFromRects(moving_rect, target_rect);
            if (side == CONTACT_BOTTOM)
            {
                onFloor = true;
            }
            notifyContact(moving, target, side, hitEnemiesAbove);
        }

        bool isOnFloor() const
        {
            return onFloor;
        }

    private:
        Object *moving;
        std::function<void(Object *)> hitEnemiesAbove;
        bool onFloor;
    };
}

void Physics::collision(Object *obj,
                        const std::vector<CollisionBody> &collisionBodies,
                        const std::vector<Object *> &dynamicObjects,
                        const std::function<void(Object *)> &hitEnemiesAbove)
{
    if (obj->dead)
    {
        return;
    }

    obj->notifyFreeLeft();
    obj->notifyFreeRight();

    Rectangle moving_rect = objectRect(obj);
    b2World box2dWorld(b2Vec2(0.0f, 0.0f));

    attachFixture(box2dWorld, obj, inflateRect(moving_rect, CONTACT_SKIN), b2_dynamicBody);

    for (auto &body : collisionBodies)
    {
        if (!expandedBoundsOverlap(moving_rect, body.bounds))
        {
            continue;
        }

        for (auto &part : body.parts)
        {
            if (!part.object->dead)
            {
                attachFixture(box2dWorld, part.object, part.rect, b2_staticBody);
                notifyDistances(obj, moving_rect, part.rect);
            }
        }
    }

    for (auto target : dynamicObjects)
    {
        if (target == obj || target->dead)
        {
            continue;
        }

        Rectangle target_rect = objectRect(target);
        attachFixture(box2dWorld, target, target_rect, b2_staticBody);
        notifyDistances(obj, moving_rect, target_rect);
    }

    CollisionContactListener listener(obj, hitEnemiesAbove);
    box2dWorld.SetContactListener(&listener);
    box2dWorld.Step(1.0f / 60.0f, 8, 3);

    if (!listener.isOnFloor())
    {
        obj->notifyFreeBottom();
    }
}
