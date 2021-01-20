#pragma once

#include "BasicGraphicsScene.hpp"
#include "DataFlowGraphModel.hpp"
#include "DataModelRegistry.hpp"
#include "Export.hpp"


namespace QtNodes
{

class NODE_EDITOR_PUBLIC DataFlowGraphicsScene
  : public BasicGraphicsScene
{
  Q_OBJECT
public:

  DataFlowGraphicsScene(DataFlowGraphModel & graphModel,
                        std::shared_ptr<DataModelRegistry> modelRegistry,
                        QObject * parent = nullptr);

  ~DataFlowGraphicsScene();

public:

  DataModelRegistry &
  registry() const;

  void
  setRegistry(std::shared_ptr<DataModelRegistry> registry);

public:

  std::vector<NodeId>
  selectedNodes() const;

public:


  QMenu *
  createSceneMenu() const override;


public Q_SLOTS:

  void
  save() const;

  void
  load();


  //std::shared_ptr<Connection> restoreConnection(QJsonObject const & connectionJson);

  //Node & restoreNode(QJsonObject const & nodeJson)


  //void save() const;

  //void load();

  //QByteArray saveToMemory() const;

  //void loadFromMemory(const QByteArray & data);

private:

  DataFlowGraphModel & _graphModel;

  std::shared_ptr<DataModelRegistry> _modelRegistry;

};

}
