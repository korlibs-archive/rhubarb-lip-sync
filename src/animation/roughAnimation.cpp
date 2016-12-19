#include "roughAnimation.h"
#include <boost/optional.hpp>

using boost::optional;

// Create timeline of shapes using a bidirectional algorithm.
// Here's a rough sketch:
//
// * Most consonants result in shape sets with multiple options; most vowels have only one shape option.
// * When speaking, we tend to slur mouth shapes into each other. So we animate from start to end,
//   always choosing a shape from the current set that resembles the last shape and is somewhat relaxed.
// * When speaking, we anticipate vowels, trying to form their shape before the actual vowel.
//   So whenever we come across a one-shape vowel, we backtrack a little, spreating that shape to the left.
JoiningContinuousTimeline<Shape> animateRough(const ContinuousTimeline<ShapeRule>& shapeRules) {
	JoiningContinuousTimeline<Shape> shapes(shapeRules.getRange(), Shape::X);

	Shape referenceShape = Shape::X;
	// Animate forwards
	centiseconds lastAnticipatedShapeStart = -1_cs;
	for (auto it = shapeRules.begin(); it != shapeRules.end(); ++it) {
		const ShapeRule shapeRule = it->getValue();
		const ShapeSet shapeSet = std::get<ShapeSet>(shapeRule);
		const Shape shape = getClosestShape(referenceShape, shapeSet);
		shapes.set(it->getTimeRange(), shape);
		const auto phone = std::get<optional<Phone>>(shapeRule);
		const bool anticipateShape = phone && isVowel(*phone) && shapeSet.size() == 1;
		if (anticipateShape) {
			// Animate backwards a little
			const Shape anticipatedShape = shape;
			const centiseconds anticipatedShapeStart = it->getStart();
			referenceShape = anticipatedShape;
			for (auto reverseIt = it; reverseIt != shapeRules.begin(); ) {
				--reverseIt;

				// Make sure we haven't animated too far back
				centiseconds anticipatingShapeStart = reverseIt->getStart();
				if (anticipatingShapeStart == lastAnticipatedShapeStart) break;
				const centiseconds maxAnticipationDuration = 20_cs;
				const centiseconds anticipationDuration = anticipatedShapeStart - anticipatingShapeStart;
				if (anticipationDuration > maxAnticipationDuration) break;

				// Make sure the new, backwards-animated shape still resembles the anticipated shape
				const Shape anticipatingShape = getClosestShape(referenceShape, std::get<ShapeSet>(reverseIt->getValue()));
				if (getBasicShape(anticipatingShape) != getBasicShape(anticipatedShape)) break;

				// Overwrite forward-animated shape with backwards-animated, anticipating shape
				shapes.set(reverseIt->getTimeRange(), anticipatingShape);

				referenceShape = anticipatingShape;
			}
			lastAnticipatedShapeStart = anticipatedShapeStart;
		}
		referenceShape = anticipateShape ? shape : relax(shape);
	}

	return shapes;
}