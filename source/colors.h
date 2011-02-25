#ifndef __COLORS_H__
#define __COLORS_H__

#include "vmath.h"
#include "common.h"

static const Vector Red     (1.0f, 0.0f, 0.0f);
static const Vector Green   (0.0f, 1.0f, 0.0f);
static const Vector Blue    (0.0f, 0.0f, 1.0f);
static const Vector Black   (0.0f, 0.0f, 0.0f);
static const Vector White   (1.0f, 1.0f, 1.0f);
static const Vector Yellow  (1.0f, 1.0f, 0.0f);
static const Vector Cyan    (0.0f, 1.0f, 1.0f);
static const Vector Magenta (1.0f, 0.0f, 1.0f);
static const Vector Grey    (0.5f, 0.5f, 0.5f);
static const Vector Pink    (1.0f, 0.2f, 0.4f);
static const Vector Orange  (1.0f, 0.5f, 0.0f);
static const Vector Violet  (0.5f, 0.0f, 0.5f);

inline Vector brighter(const Vector& color, float times = 2.0f)
{
    return color * times;
}

inline Vector darker(const Vector& color, float times = 2.0f)
{
    return color / times;
}

#endif
