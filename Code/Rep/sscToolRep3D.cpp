#include "sscToolRep3D.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>

#include "sscToolManager.h"
#include "sscTool.h"
#include "sscView.h"

namespace ssc
{

ToolRep3D::ToolRep3D(const std::string& uid, const std::string& name) :
	RepImpl(uid, name)
{
	mToolActor = vtkActorPtr::New();
	mPolyDataMapper = vtkPolyDataMapperPtr::New();
	mSTLReader = vtkSTLReaderPtr::New();
	mOffsetPoint.reset(new GraphicalPoint3D());
	mOffsetLine.reset(new GraphicalLine3D());	
	
//	if (pd::Settings::instance()->useDebugAxis())
//	 	{
//		 	mTool->AddPart( Axes3D().getProp() );
//	 	}
}
	
ToolRep3D::~ToolRep3D()
{}
ToolRep3DPtr ToolRep3D::New(const std::string& uid, const std::string& name)
{
	ToolRep3DPtr retval(new ToolRep3D(uid, name));
	retval->mSelf = retval;
	return retval;
}

std::string ToolRep3D::getType() const
{
	return "ssc::ToolRep3D";
}

ToolPtr ToolRep3D::getTool()
{
	return mTool;
}

void ToolRep3D::setTool(ToolPtr tool)
{
	if (tool==mTool)
		return;
	
	// teardown old
	if (mTool)
	{
		disconnect(mTool.get(), SIGNAL(toolTransformAndTimestamp(Transform3D, double)),
				this, SLOT(receiveTransforms(Transform3D, double)));

		disconnect(mTool.get(), SIGNAL(toolVisible(bool)),
				this, SLOT(receiveVisible(bool)));
		disconnect(mTool.get(), SIGNAL(tooltipOffset(double)), this, SLOT(tooltipOffsetSlot(double)));		
		
		mToolActor->SetMapper(NULL);
	}
	
	mTool = tool;
	
	// setup new
	if (mTool)
	{
		std::string filename = mTool->getGraphicsFileName();
		//std::cout<<"reading filename :" << filename <<	std::endl;
		if (!filename.empty() && filename.compare(filename.size()-3,3,"STL") == 0 )
		{
			mSTLReader->SetFileName( filename.c_str() ); 
			mPolyDataMapper->SetInputConnection( mSTLReader->GetOutputPort() );	 //read a 3D model file of the tool			
		}
		else 
		{
			mPolyDataMapper->SetInput( mTool->getGraphicsPolyData() ); // creates a cone, default
		}
		
		if (mPolyDataMapper->GetInput())
		{
			mToolActor->SetMapper(mPolyDataMapper);
		}
	
		receiveTransforms(mTool->get_prMt(), 0);
		mToolActor->SetVisibility(mTool->getVisible());
		
		connect(mTool.get(), SIGNAL(toolTransformAndTimestamp(Transform3D, double)),
				this, SLOT(receiveTransforms(Transform3D, double)));
		connect(mTool.get(), SIGNAL(toolVisible(bool)),
				this, SLOT(receiveVisible(bool)));
		connect(mTool.get(), SIGNAL(tooltipOffset(double)), this, SLOT(tooltipOffsetSlot(double)));		
		
	}
}

bool ToolRep3D::hasTool(ToolPtr tool) const
{
	return (mTool != NULL);
}

void ToolRep3D::addRepActorsToViewRenderer(View* view)
{
	view->getRenderer()->AddActor(mToolActor);
	
	mOffsetPoint.reset(new GraphicalPoint3D(view->getRenderer()));
	mOffsetPoint->setRadius(3);
	mOffsetPoint->setColor(Vector3D(1,1,0));
	mOffsetLine.reset(new GraphicalLine3D(view->getRenderer()));	
	mOffsetLine->setColor(Vector3D(1,1,0));
}

void ToolRep3D::removeRepActorsFromViewRenderer(View* view)
{
	view->getRenderer()->RemoveActor(mToolActor);
	mOffsetPoint.reset(new GraphicalPoint3D());
	mOffsetLine.reset(new GraphicalLine3D());	
}

void ToolRep3D::receiveTransforms(Transform3D prMt, double timestamp)
{
	//return;
	Transform3D rMpr = *ssc::ToolManager::getInstance()->get_rMpr();
	
//	std::cout << "--ToolRep3D::receiveTransforms()---" << std::endl;
//	std::cout << "prMt\n" << prMt << std::endl;
//	std::cout << "rMpr\n" << rMpr << std::endl;
//	std::cout << "----------" << std::endl;
	
	Transform3D rMt = rMpr*prMt;	
	mToolActor->SetUserMatrix( rMt.matrix());

	updateOffsetGraphics();
}

void ToolRep3D::updateOffsetGraphics()
{
	bool visible = mTool && !similar(0.0, mTool->getTooltipOffset()) && mTool->getVisible();
	mOffsetPoint->getActor()->SetVisibility(visible);
	mOffsetLine->getActor()->SetVisibility(visible);
	
	if (!mTool)
		return;
	
	Transform3D rMpr = *ssc::ToolManager::getInstance()->get_rMpr();
	Transform3D rMt = rMpr * mTool->get_prMt();	

	Vector3D p0 = rMt.coord(Vector3D(0,0,0));
	Vector3D p1 = rMt.coord(Vector3D(0,0,mTool->getTooltipOffset()));	
	mOffsetPoint->setValue(p1);
	mOffsetLine->setValue(p0,p1);	
}

void ToolRep3D::receiveVisible(bool visible)
{
	//std::cout << "ToolRep3D::receiveVisible " << visible << std::endl;
	if (!visible && mStayVisibleAfterHide)
	{
		return; // don't hide
	}
	
	mToolActor->SetVisibility(visible);
	updateOffsetGraphics();
}

/**
 * If true, tool is still rendered as visible after visibility status is hidden.
 * Nice for viewing the last known position of a tool.
 */
void ToolRep3D::setStayVisibleAfterHide(bool val)
{
	mStayVisibleAfterHide = val;	
}

void ToolRep3D::tooltipOffsetSlot(double val)
{
	updateOffsetGraphics();	
}

} // namespace ssc
