#ifndef SSCMESH_
#define SSCMESH_

#include <set>
#include <boost/shared_ptr.hpp>

#include "vtkForwardDeclarations.h"

#include <QColor>
#include "sscData.h"

class QDomNode;
class QDomDocument;

namespace ssc
{

/**Defines a geometric structure in space..?
 * 
 */
class Mesh : public Data
{
  Q_OBJECT
public:
	Mesh(const QString& uid, const QString& name="");
	Mesh(const QString& uid, const QString& name, const vtkPolyDataPtr& polyData);
	virtual ~Mesh();

	void setVtkPolyData(const vtkPolyDataPtr& polyData);

	virtual vtkPolyDataPtr getVtkPolyData();
  
	void addXml(QDomNode& dataNode); ///< adds xml information about the image and its variabels
	virtual void parseXml(QDomNode& dataNode);///< Use a XML node to load data. \param dataNode A XML data representation of this object.
  virtual QString getType() const { return "mesh"; }

	virtual DoubleBoundingBox3D boundingBox() const;
  void setColor(const QColor& color);///< Set the color of the mesh
  QColor getColor();///< Get the color of the mesh (Values are range 0 - 255)
  vtkPolyDataPtr getTransformedPolyData(ssc::Transform3D tranform);///< Create a new transformed polydata
  
signals:
  void meshChanged();
private:
	vtkPolyDataPtr mVtkPolyData;
  QColor mColor;
};

typedef boost::shared_ptr<Mesh> MeshPtr;

} // namespace ssc

#endif /*SSCMESH_*/
