#include <iostream>
#include <cmath>
#include <vector>

namespace Geometry {
    bool equal(double first, double second, double eps) {
        return fabs(first - second) < eps;
    }
}

struct Point;

class Line {
public:
    Line(): a(1), b(-1), c(0) {}

    Line(const Point& first, const Point& second);

    Line(const double& m, const double& dist);

    Line(const Point& v, const double& m);

    Line perp(const Point& v) const;

    Point operator*(const Line& other) const;

    double get_coef() const;

    double dist(const Point& p) const;

    bool operator==(const Line& other) const;

    void norm();

    double a;
    double b;
    double c;

    static const constexpr double pi = 3.141592653589793238;
    static const constexpr double eps = 1e-9;
};
/////////////////////////////////////////////////
struct Point {
    double x;
    double y;
    static const constexpr double pi = 3.141592653589793238;
    static const constexpr double eps = 1e-7;

    Point() : x(0), y(0) {}

    Point(double x_, double y_) : x(x_), y(y_) {}

    Point(const Point &other): x(other.x), y(other.y) {}

    Point& operator=(const Point &other) = default;

    bool operator==(const Point &other) const {
        return Geometry::equal(x, other.x, eps) && Geometry::equal(y, other.y, eps);
    }

    Point operator+(const Point &other) const {
        return Point{x + other.x, y + other.y};
    }

    Point operator-(const Point &other) const {
        return Point{x - other.x, y - other.y};
    }

    Point& operator+=(const Point &other) {
        return *this = *this + other;
    }

    Point& operator-=(const Point &other) {
        return *this = *this - other;
    }

    Point operator/(const double &dec) const {
        return *this * (1 / dec);
    }

    double dotProduct(const Point &other) const {
        return x * other.x + y * other.y;
    }

    double crossProduct(const Point &other) const {
        return x * other.y - y * other.x;
    }

    double len() const {
        return sqrt(x * x + y * y);
    }

    Point operator*(double t) const {
        return Point{x * t, y * t};
    }

    void reflect(const Point& p) {
        *this -= (*this - p) * 2;
    }

    void reflect(const Line& l) {
        Point center = l * l.perp(*this);
        reflect(center);
    }

    void rotate(const Point& center, double angle) {
        Point crt = *this - center;
        angle /= 180 / pi;
        Point res{crt.x * cos(angle) - crt.y * sin(angle), crt.x * sin(angle) + crt.y * cos(angle)};
        *this = center + res;
    }

    void scale(const Point& center, double coefficient) {
        *this = center + (*this - center) * coefficient;
    }
};


/////////////////////////////////////////////////////////////////////////////////

void Line::norm() {
    double g = std::sqrt(a * a + b * b);
    a /= g;
    b /= g;
    c /= g;
    if (a < 0) {
        a *= -1;
        b *= -1;
        c *= -1;
    }
}

double Line::get_coef() const {
    return -a / b;
}

Line::Line(const double& m, const double& dist): a(1), b(-a / m), c(dist * sqrt(a * a + b * b)) {}

Line::Line(const Point& first, const Point& second) {
    a = second.y - first.y;
    b = first.x - second.x;
    c = second.x * first.y - second.y * first.x;
}

Point Line::operator*(const Line& other) const {
    Point res;
    res.y = (a * other.c - other.a * c) / (other.a * b - a * other.b);
    res.x = -(b * other.c - other.b * c) / (other.a * b - a * other.b);
    return res;
}

Line::Line(const Point& v, const double& m): a(-m), b(1), c(m * v.x - v.y) {}

double Line::dist(const Point& p) const {
    return fabs(a * p.x + b * p.y + c) / sqrt(a * a + b * b);
}

Line Line::perp(const Point& v) const {
    Line res;
    res.a = -b;
    res.b = a;
    res.c = -(v.x * res.a + v.y * res.b);
    return res;
}

bool Line::operator==(const Line& other) const {
    Line first = *this;
    Line second = other;
    first.norm();
    second.norm();
    return Geometry::equal(first.a, second.a, eps) && Geometry::equal(first.b, second.b, eps)
            && Geometry::equal(first.c, second.c, eps);
}

///////////////////////////////////////////////////////////////////////////////////////
class Shape {
public:
    Shape() = default;

    virtual double perimeter() const = 0;

    virtual double area() const = 0;

    virtual bool operator==(const Shape& other) const = 0;

    virtual bool isCongruentTo(const Shape& another) const = 0;

    virtual bool isSimilarTo(const Shape& another) const = 0;

    virtual bool containsPoint(const Point& point) const = 0;

    virtual void rotate(const Point& center, double angle) = 0;

    virtual void reflect(const Point& center) = 0;

    virtual void reflect(const Line& axis) = 0;

    virtual void scale(const Point& center, double coefficient) = 0;

    virtual ~Shape() = default;

    static const constexpr double pi = 3.141592653589793238;
    static const constexpr double eps = 1e-7;
};
////////////////////////////////////////////////////////////////////////

class Ellipse : public Shape {
public:
    Ellipse() = default;

    Ellipse(const Point& a, const Point& b, double sum);

    std::pair<Point, Point> focuses() const;

    std::pair<Line, Line> directrices() const;

    double eccentricity() const;

    Point center() const;

    bool containsPoint(const Point& point) const override;

    void reflect(const Point& center) override;

    void reflect(const Line& axis) override;

    void rotate(const Point& center, double angle) override;

    void scale(const Point& center, double coefficient) override;

    bool operator==(const Shape& other) const override;

    virtual bool isCongruentTo(const Shape& another) const override;

    bool isSimilarTo(const Shape& another) const override;

    ~Ellipse() override = default;

    double area() const override;

    double perimeter() const override;

protected:
    Point first_focus;
    Point second_focus;
    double long_axis;
    double eccentricity_;
};

Ellipse::Ellipse(const Point& ff, const Point& sf, double sum): first_focus(ff), second_focus(sf)
        , long_axis(sum / 2), eccentricity_((ff - sf).len() / sum) {}

double Ellipse::area() const {
    double focus_dist = (first_focus - second_focus).len() / 2;
    double short_axis = sqrt(long_axis * long_axis - focus_dist * focus_dist);
    return pi * long_axis * short_axis;
}

double Ellipse::perimeter() const {
    double focus_dist = (first_focus - second_focus).len() / 2;
    double short_axis = sqrt(long_axis * long_axis - focus_dist * focus_dist);
    return pi * (3 * (long_axis + short_axis) - sqrt((3 * long_axis + short_axis) * (long_axis + 3 * short_axis)));
}

std::pair<Point, Point> Ellipse::focuses() const {
    return std::make_pair(first_focus, second_focus);
}

std::pair<Line, Line> Ellipse::directrices() const {
    Line axis = Line(first_focus, second_focus);
    Line first_directrice = Line(first_focus, -1 /  axis.get_coef());
    Line second_directrice = Line(second_focus, -1 /  axis.get_coef());
    return std::make_pair(first_directrice, second_directrice);
}

double Ellipse::eccentricity() const {
    return eccentricity_;
}

Point Ellipse::center() const {
    return (first_focus + second_focus) / 2;
}

bool Ellipse::containsPoint(const Point& point) const {
    return (point - first_focus).len() + (point - second_focus).len() <= 2 * long_axis + eps;
}

void Ellipse::reflect(const Point& center) {
    first_focus.reflect(center);
    second_focus.reflect(center);
}

void Ellipse::reflect(const Line& axis) {
    first_focus.reflect(axis);
    second_focus.reflect(axis);
}

void Ellipse::rotate(const Point& center, double angle) {
    first_focus.rotate(center, angle);
    second_focus.rotate(center, angle);
}

void Ellipse::scale(const Point& center, double coefficient) {
    first_focus.scale(center, coefficient);
    second_focus.scale(center, coefficient);
    long_axis *= fabs(coefficient);
}

bool Ellipse::operator==(const Shape& other) const {
    const Ellipse* ptr = dynamic_cast<const Ellipse*>(&other);
    if (ptr == nullptr) {
        return false;
    }
    if (focuses() != ptr->focuses() && ptr->focuses() != std::make_pair(second_focus, first_focus))
        return false;
    return Geometry::equal(eccentricity_, ptr->eccentricity_, eps);
}

bool Ellipse::isCongruentTo(const Shape& another) const {
    const Ellipse* ptr = dynamic_cast<const Ellipse*>(&another);
    if (ptr == nullptr) {
        return false;
    }
    return Geometry::equal(long_axis, ptr->long_axis, eps) && Geometry::equal(eccentricity_, ptr->eccentricity_, eps);
}

bool Ellipse::isSimilarTo(const Shape& another) const {
    const Ellipse* ptr = dynamic_cast<const Ellipse*>(&another);
    if (ptr == nullptr) {
        return false;
    }
    return Geometry::equal(eccentricity_, ptr->eccentricity_, eps);
}

///////////////////////////////////////////////////////////////////////////////////////

class Circle : public Ellipse {
public:
    Circle(const Point& center, double r) {
        first_focus = second_focus = center;
        long_axis = r;
        eccentricity_ = 0;
    }
    double radius() { return long_axis; }
    bool operator==(const Circle& other) const {
        return first_focus == other.first_focus && Geometry::equal(long_axis, other.long_axis, eps);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////
class Polygon : public Shape {
public:
    Polygon() = default;

    template <typename... Args>
    explicit Polygon(Args... args) : points({args...}), is_convex(check_convex(points)) {}

    Polygon(const std::vector<Point>& v): points(v), is_convex(check_convex(points)) {}

    Polygon(const std::initializer_list<Point>& vert):  points(vert), is_convex(check_convex(points)) {}

    size_t verticesCount() const;

    const std::vector<Point>& getVertices() const;

    bool isConvex() const;

    bool containsPoint(const Point& point) const override;

    void reflect(const Point& center) override;

    void reflect(const Line& axis) override;

    void rotate(const Point& center, double angle) override;

    void scale(const Point& center, double coefficient) override;

    bool operator==(const Shape& other) const override;

    virtual bool isCongruentTo(const Shape& another) const override;

    bool isSimilarTo(const Shape& another) const override;

    bool operator==(const Polygon& other) const;

    ~Polygon() override = default;

    double perimeter() const override;

    double area() const override;

protected:
    std::vector <Point> points;
    bool is_convex;

    static bool check_convex(std::vector<Point>& v);

    bool check_similar_angles(const std::vector<Point>& other_polygon, size_t first_begin, size_t second_begin, int step) const;
};

size_t Polygon::verticesCount() const { return points.size(); }

bool Polygon::check_convex(std::vector<Point> &v) {
    int cnt_left = 0, cnt_right = 0;
    v.push_back(v[0]);
    v.push_back(v[1]);
    for (size_t i = 1; i < v.size() - 1; ++i) {
        if ((v[i] - v[i - 1]).crossProduct(v[i + 1] - v[i]) > 0) {
            ++cnt_left;
        } else {
            ++cnt_right;
        }
    }
    v.pop_back();
    v.pop_back();
    return cnt_right == 0 || cnt_left == 0;
}

const std::vector<Point>& Polygon::getVertices() const { return points; }

bool Polygon::isConvex() const { return is_convex; }

double Polygon::perimeter() const {
    double result = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        result += (points[i] - points[i - 1]).len();
    }
    result += (points.back() - points[0]).len();
    return result;
}

double Polygon::area() const {
    double result = 0;
    for (size_t i = 2; i < points.size(); ++i) {
        result += (points[i] - points[0]).crossProduct(points[i - 1] - points[0]);
    }
    return fabs(result) / 2;
}

bool Polygon::containsPoint(const Point& point) const {
    double angle = 0;
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        if (fabs((points[i] - point).crossProduct(points[i + 1] - point)) < eps
            && (points[i] - point).dotProduct(points[i + 1] - point) <= 0) return true;
    }
    if (fabs((points.back() - point).crossProduct(points[0] - point)) < eps
        && (points.back() - point).dotProduct(points[0] - point) <= 0) return true;

    for (size_t i = 0; i + 1 < points.size(); ++i) {
        angle += atan2((points[i] - point).crossProduct(points[i + 1] - point),
                       (points[i] - point).dotProduct(points[i + 1] - point));
    }
    angle += atan2((points.back() - point).crossProduct(points[0] - point),
                   (points.back() - point).dotProduct(points[0] - point));
    if (fabs(angle) < 1) return false;
    return true;
}

void Polygon::reflect(const Point& center) {
    for (Point& p : points) {
        p.reflect(center);
    }
}

void Polygon::reflect(const Line& axis) {
    for (Point& p : points) {
        p.reflect(axis);
    }
}

void Polygon::rotate(const Point& center, double angle) {
    for (Point& p : points) {
        p.rotate(center, angle);
    }
}

void Polygon::scale(const Point& center, double coefficient) {
   for (Point& p : points) {
       p.scale(center, coefficient);
   }
   is_convex = check_convex(points);
}

bool Polygon::operator==(const Shape& other) const {
    const Polygon* ptr = dynamic_cast<const Polygon*>(&other);
    if (ptr == nullptr) return false;
    return *this == *ptr;
}

bool Polygon::isCongruentTo(const Shape& another) const {
    return fabs(perimeter() - another.perimeter()) < eps && isSimilarTo(another);
}

bool Polygon::isSimilarTo(const Shape& another) const {
    const Polygon* ptr = dynamic_cast<const Polygon*>(&another);
    if (ptr == nullptr) return false;
    if (verticesCount() != ptr->verticesCount() || is_convex != ptr->is_convex) return false;
    size_t len = verticesCount();
    for (size_t i = 0; i < verticesCount(); ++i) {
        bool similar_left = true;
        bool similar_right = true;
        for (size_t j = 0; j < verticesCount(); ++j) {
            similar_left &= check_similar_angles(ptr->points, j, (j + i) % len, 1);
            similar_right &= check_similar_angles(ptr->points, j, (i + len - j) % len, -1);
        }
        if (similar_left || similar_right) return true;
    }
    return false;
}

bool Polygon::check_similar_angles(const std::vector<Point>& other_polygon, size_t first_begin, size_t second_begin, int step) const {
    size_t len = points.size();
    Point a = points[first_begin];
    Point b = points[(first_begin + 1) % len];
    Point c = points[(first_begin + 2) % len];
    Point x = other_polygon[second_begin];
    Point y = other_polygon[(second_begin + step + len) % len];
    Point z = other_polygon[(second_begin + step * 2 + len) % len];
    if (fabs((b - a).len() / (y - x).len() - (c - b).len() / (z - y).len()) > eps) return false;
    if (fabs(atan2((b - a).crossProduct(c - b), (b - a).dotProduct(c - b))
             - atan2((b - a).crossProduct(c - b), (b - a).dotProduct(c - b))) > eps) return false;
    return true;
}

bool Polygon::operator==(const Polygon& other) const {
    if (verticesCount() != other.verticesCount()) return false;
    for (size_t i = 0; i < verticesCount(); ++i) {
        bool equal_left = true;
        bool equal_right = true;
        for (size_t j = 0; j < verticesCount(); ++j) {
            equal_left = points[j] == other.points[(i + j) % verticesCount()];
            equal_right = points[j] == other.points[(i + verticesCount() - j) % verticesCount()];
        }
        if (equal_left || equal_right) return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////

class Rectangle : public Polygon {
public:
    Rectangle(const Point& first, const Point& second, double c);

    Point center();

    std::pair<Line, Line> diagonals();
};

Rectangle::Rectangle(const Point& first, const Point& second, double c) {
    if (c > 1) c = 1 / c;
    double angle = atan(c) / pi * 180;
    double b = sqrt(pow((first - second).len(), 2) / (c * c + 1));
    Point left = first;
    left.rotate(second, -angle);
    left = left * (b / (first - second).len());
    Point right = second;
    right.rotate(first, -angle);
    right = right * (b / (first - second).len());
    points.resize(4);
    points = {first, left, second, right};
    is_convex = true;
}

Point Rectangle::center() {
    std::pair<Line, Line> diagonals_ = diagonals();
    return diagonals_.first * diagonals_.second;
}

std::pair<Line, Line> Rectangle::diagonals() {
    return std::make_pair(Line(points[0], points[2]), Line(points[1], points[3]));
}

//////////////////////////////////////////////////////////////////////////////////////
class Square : public Rectangle {
public:
    Square(const Point& a, const Point& b): Rectangle(a, b, 1) {}

    Circle circumscribedCircle() { return {center(), (points[0] - points[2]).len() / 2}; }

    Circle inscribedCircle() { return {center(), (points[0] - points[1]).len() / 2}; }
};
//////////////////////////////////////////////////////////////////////////////////////
class Triangle : public Polygon {
public:
    using Polygon::Polygon;

    Circle circumscribedCircle() const;

    Circle inscribedCircle() const;

    Point centroid() const;

    Point orthocenter() const;

    Line EulerLine() const;

    Circle ninePointsCircle() const;
private:
    Line bis(Line a, Line b) const;
};

Point Triangle::centroid() const{
    return (points[0] + points[1] + points[2]) / 3;
}

Line Triangle::bis(Line a, Line b) const{
    a.norm();
    b.norm();
    Line res;
    res.a = a.a + b.a;
    res.b = a.b + b.b;
    res.c = a.c + b.c;
    return res;
}

Circle Triangle::inscribedCircle() const {
    Line first = bis(Line(points[1], points[0]), Line(points[2], points[0]));
    Line second = bis(Line(points[0], points[1]), Line(points[2], points[1]));
    Point center = first * second;
    return {center, Line(points[0], points[1]).dist(center)};
}

Circle Triangle::circumscribedCircle() const {
    Line first = Line(points[0], points[1]).perp((points[0] + points[1]) / 2);
    Line second = Line(points[2], points[1]).perp((points[2] + points[1]) / 2);
    Point center = first * second;
    return {center, (center - points[0]).len()};
}

Circle Triangle::ninePointsCircle() const {
    Triangle small((points[0] + points[1]) / 2, (points[2] + points[1]) / 2, (points[0] + points[2]) / 2);
    return small.circumscribedCircle();
}

Point Triangle::orthocenter() const {
    Line first = Line(points[0], points[1]).perp(points[2]);
    Line second = Line(points[0], points[2]).perp(points[1]);
    return first * second;
}

Line Triangle::EulerLine() const {
    return {centroid(), orthocenter()};
}
