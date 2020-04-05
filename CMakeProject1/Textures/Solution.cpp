#include "Common.h"

using namespace std;

class Figure: public IShape {
public:
    virtual ~Figure() = default;

    Figure(shared_ptr<ITexture> texture, Size size, Point point, ShapeType type)
        : texture(texture)
        , size(size)
        , point(point)
        , type(type)
    {}

    Figure(ShapeType type)
        : type(type)
        , texture(nullptr)
    {}

    virtual std::unique_ptr<IShape> Clone() const override {
        return make_unique<Figure>(texture, size, point, type);
    }

    virtual void SetPosition(Point point_) override {
        point = point_;
    }

    virtual Point GetPosition() const override {
        return point;
    }

    virtual void SetSize(Size size_) override {
        size = size_;
    }

    virtual Size GetSize() const override {
        return size;
    }

    virtual void SetTexture(std::shared_ptr<ITexture> texture_) override {
        texture = texture_;
    }

    virtual ITexture* GetTexture() const override {
        return texture.get();
    }

    virtual void Draw(Image& image) const override {
        for (int i = 0; i < size.height; ++i) {
            for (int j = 0; j < size.width; ++j) {
                if (type == ShapeType::Ellipse && !IsPointInEllipse(Point{ j, i }, size)) {
                    continue;
                }
                int y = i + point.y;
                int x = j + point.x;
                if (!(y < image.size() && x < image[y].size())) continue;
                if (texture && i < texture->GetSize().height && j < texture->GetSize().width) {
                    image[y][x] = texture->GetImage()[i][j];
                }
                else {
                    image[y][x] = '.';
                }
            }
        }
    }

private:
    shared_ptr<ITexture> texture;
    Size size;
    Point point;
    ShapeType type;
};



// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    return make_unique<Figure>(shape_type);
}