#pragma once

#include <set>
#include <boost/optional.hpp>
#include "Shape.h"
#include "Timeline.h"
#include "Phone.h"

// A set of mouth shapes that can be used to represent a certain sound
using ShapeSet = std::set<Shape>;

Timeline<ShapeSet> animatePhone(boost::optional<Phone> phone, centiseconds duration, centiseconds previousDuration);
