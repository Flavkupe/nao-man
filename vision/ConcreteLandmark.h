#ifndef ConcreteLandmark_h_defined__
#define ConcreteLandmark_h_defined__

#include <string>
using namespace std;

class ConcreteLandmark {
public:
  ConcreteLandmark(const float _fieldX, const float _fieldY);
  ConcreteLandmark(const ConcreteLandmark& other);
  virtual ~ConcreteLandmark();

  virtual const string toString() const = 0;

  const float getFieldX() const { return fieldX; }
  const float getFieldY() const { return fieldY; }

private:
  //  point <const float> fieldLocation;
  const float fieldX, fieldY;
};

#endif
