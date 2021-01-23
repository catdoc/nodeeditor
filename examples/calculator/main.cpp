#include <nodes/ConnectionStyle>
#include <nodes/DataFlowGraphModel>
#include <nodes/DataFlowGraphicsScene>
#include <nodes/DataModelRegistry>
#include <nodes/GraphicsView>
#include <nodes/NodeData>

#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMenuBar>

#include "NumberSourceDataModel.hpp"
#include "NumberDisplayDataModel.hpp"
#include "AdditionModel.hpp"
#include "SubtractionModel.hpp"
#include "MultiplicationModel.hpp"
#include "DivisionModel.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphModel;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataModelRegistry;
using QtNodes::GraphicsView;

static std::shared_ptr<DataModelRegistry>
registerDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<NumberSourceDataModel>("Sources");

  ret->registerModel<NumberDisplayDataModel>("Displays");

  ret->registerModel<AdditionModel>("Operators");

  ret->registerModel<SubtractionModel>("Operators");

  ret->registerModel<MultiplicationModel>("Operators");

  ret->registerModel<DivisionModel>("Operators");

  //ret->registerTypeConverter(std::make_pair(DecimalData().type(),
  //IntegerData().type()),
  //TypeConverter{DecimalToIntegerConverter()});

  //ret->registerTypeConverter(std::make_pair(IntegerData().type(),
  //DecimalData().type()),
  //TypeConverter{IntegerToDecimalConverter()});

  return ret;
}


static
void
setStyle()
{
  ConnectionStyle::setConnectionStyle(
    R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "black",
      "SelectedColor": "gray",
      "SelectedHaloColor": "deepskyblue",
      "HoveredColor": "deepskyblue",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": true
    }
  }
  )");
}


int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  setStyle();

  std::shared_ptr<DataModelRegistry> registry = registerDataModels();

  QWidget mainWidget;

  auto menuBar    = new QMenuBar();
  auto saveAction = menuBar->addAction("Save..");
  auto loadAction = menuBar->addAction("Load..");

  QVBoxLayout *l = new QVBoxLayout(&mainWidget);

  DataFlowGraphModel dataFlowGraphModel(registry);

  l->addWidget(menuBar);
  auto scene = new DataFlowGraphicsScene(dataFlowGraphModel,
                                         &mainWidget);
  l->addWidget(new GraphicsView(scene));
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);

  QObject::connect(saveAction, &QAction::triggered,
                   scene, &DataFlowGraphicsScene::save);

  QObject::connect(loadAction, &QAction::triggered,
                   scene, &DataFlowGraphicsScene::load);

  mainWidget.setWindowTitle("Dataflow tools: simplest calculator");
  mainWidget.resize(800, 600);
  mainWidget.showNormal();

  return app.exec();
}

