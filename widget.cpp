#include "widget.h"

#include <QGridLayout>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

#include <random>

#define N 32

Widget::Widget(QWidget* parent) :
  QWidget(parent)
{
  cells.reserve(N*N);

  setup();
  play();
}

void Widget::setup()
{
  for (int i = 0; i < 2; i++)
  {
    score[i] = new QLabel("0", this);
    score[i]->setFixedHeight(20);
    score[i]->setAlignment(Qt::AlignCenter);

    buttonLayout[i] = new QVBoxLayout;
    buttonLayout[i]->addWidget(score[i]);

    int k = 0;
    for (const QColor& color : colors)
    {
      QPushButton* button = new QPushButton(this);
      button->setObjectName("button" + QString::number(i));
      button->setFixedSize(80, 80);
      button->setStyleSheet("background-color: " + color.name() + ";");
      button->setShortcut(QKeySequence(Qt::Key_1 + k++));

      connect(button, &QPushButton::pressed, [this, color, i]() { press(color, i); play(); });

      buttonLayout[i]->addWidget(button);
    }

    QPushButton* button = new QPushButton("CPU", this);

    connect(button, &QPushButton::pressed, [this, i]() { play_cpu(i); play(); });

    buttonLayout[i]->addWidget(button);
  }

  std::random_device rd;
  std::default_random_engine e1(rd());
  std::uniform_int_distribution<int> uniform_dist(0, colors.size()-1);

  QGridLayout* layout = new QGridLayout;
  layout->setSpacing(0);
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int index = uniform_dist(e1);

      QLabel* label = new QLabel(this);
      label->setFixedSize(20, 20);
      label->setAutoFillBackground(true);
      label->setPalette(QPalette(QPalette::Background, colors.at(index)));

      cells.emplace_back(label, i, j);
      layout->addWidget(label, i, j);
    }
  }

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addLayout(buttonLayout[0]);
  mainLayout->addLayout(layout);
  mainLayout->addLayout(buttonLayout[1]);

  start[0] = &cells.front();
  start[1] = &cells.back();

  for (int i = 0; i < 2; i++)
  {
    start[i]->owner = i;
    set_color(start[i]->label, QColor(i == 0 ? Qt::white : Qt::black));
  }
}

void Widget::play()
{
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < buttonLayout[i]->count(); j++)
    {
      QWidget* widget = buttonLayout[i]->itemAt(j)->widget();
      widget->setDisabled(turn);

      if (!turn)
      {
        std::string style = widget->styleSheet().toStdString();
        for (int k = 0; k < 2; k++)
        {
          if (style.find(get_color(start[k]->label).name().toStdString()) != std::string::npos)
          {
            widget->setDisabled(true);
          }
        }
      }
    }

    turn = !turn;
  }

  turn = !turn;
}

void Widget::play_cpu(int p)
{
  std::vector<Cell> orig_cells = cells;  // copy cell ownership, the other members don't count
  int orig_score = std::stoi(score[p]->text().toStdString());

  int max = 0, k = 0;
  QColor best_color;
  for (QPushButton* button : findChildren<QPushButton*>("button" + QString::number(p)))
  {
    if (button->isEnabled())
    {
      press(colors.at(k), p);

      int n = std::stoi(score[p]->text().toStdString()) - orig_score;
      if (n >= max)
      {
        max = n;
        best_color = colors.at(k);
      }

      cells = orig_cells;
    }

    k++;
  }

  press(best_color, p);
}

void Widget::press(const QColor& color, int p)
{
  std::list<Cell*> player_cells;

  for (Cell& cell : cells)
  {
    if (cell.owner == p)
    {
      player_cells.push_back(&cell);
    }
  }

  for (Cell* cell : player_cells)
  {
    QLabel* label = cell->label;
    int x = cell->x;
    int y = cell->y;

    set_color(label, color);

    std::vector< std::pair<int, int> > coords {
      std::pair<int, int>(x - 1, y),
      std::pair<int, int>(x + 1, y),
      std::pair<int, int>(x, y - 1),
      std::pair<int, int>(x, y + 1)
    };

    for (const auto& coord : coords)
    {
      Cell* c = get_cell(coord.first, coord.second);
      if (c && c->owner == -1 && get_color(c->label) == color)
      {
        c->owner = p;
        player_cells.push_back(c);
      }
    }
  }

  score[p]->setText(QString::number(player_cells.size()));

  if (finish())
  {
    QString outcome;
    if (score[0]->text().toInt() > score[1]->text().toInt())
    {
      outcome = "Player one wins!";
    }
    else if (score[0]->text().toInt() < score[1]->text().toInt())
    {
      outcome = "Player two wins!";
    }
    else
    {
      outcome = "Draw!";
    }

    QMessageBox::information(this, "Game ended", outcome);
    close();
  }
}

bool Widget::finish()
{
  for (Cell& cell : cells)
  {
    if (cell.owner == -1)
    {
      return false;
    }
  }

  return true;
}

const QColor& Widget::get_color(QLabel* label)
{
  return label->palette().color(QPalette::Background);
}

void Widget::set_color(QLabel* label, const QColor& color)
{
  label->setPalette(QPalette(color));
}

Cell* Widget::get_cell(int x, int y)
{
  Cell to_find(nullptr, x, y);
  auto it = std::find(cells.begin(), cells.end(), to_find);

  if (it != cells.end())
  {
    Cell& cell = *it;
    return &cell;
  }

  return nullptr;
}
