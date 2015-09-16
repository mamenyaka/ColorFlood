#include <QWidget>

#include <vector>

class QLabel;
class QVBoxLayout;

struct Cell {
  QLabel* label;
  int x, y;
  int owner;

  Cell(QLabel* label, int x, int y, int owner = -1) :
    label(label), x(x), y(y), owner(owner)
  {}

  bool operator==(const Cell& other)
  {
    return x == other.x && y == other.y;
  }
};

class Widget : public QWidget
{
  Q_OBJECT

  std::vector<QColor> colors = {
    QColor(Qt::yellow),
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::magenta),
    QColor(Qt::blue),
    QColor(Qt::cyan)
  };

  QVBoxLayout* buttonLayout[2];
  std::vector<Cell> cells;

  QLabel* score[2];
  Cell* start[2];
  bool turn = 0;

public:
  explicit Widget(QWidget* parent = 0);

private:
  void setup();
  void play();
  bool finish();

  void play_cpu(int p);
  void press(const QColor& color, int p);

  const QColor& get_color(QLabel* label);
  void set_color(QLabel* label, const QColor& color);
  Cell* get_cell(int x, int y);
};
