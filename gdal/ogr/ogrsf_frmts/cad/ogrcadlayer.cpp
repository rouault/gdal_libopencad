#include "ogr_cad.h"
#include "cpl_conv.h"

OGRCADLayer::OGRCADLayer( CADLayer &poCADLayer_ ) :
	poCADLayer( poCADLayer_ )
{
	nNextFID = 0;

	poFeatureDefn = new OGRFeatureDefn( CPLGetBasename( poCADLayer.getName().c_str() ) );
	SetDescription( poFeatureDefn->GetName() );
	poFeatureDefn->Reference();

	OGRFieldDefn oFieldTemplate( "Name", OFTString );

	poFeatureDefn->AddFieldDefn( &oFieldTemplate );
}

GIntBig OGRCADLayer::GetFeatureCount( int bForce )
{
    return (int)poCADLayer.getGeometryCount();
}

OGRCADLayer::~OGRCADLayer()
{
	poFeatureDefn->Release();
}

void OGRCADLayer::ResetReading()
{
	nNextFID = 0;
}


    //TODO: code duplication. Need to make something like func which will call
    // functions below with additional parameters.
OGRFeature *OGRCADLayer::GetFeature( GIntBig nFID )
{
    if( (int)poCADLayer.getGeometryCount() <= nFID )
    {
        return NULL;
    }
    
    OGRFeature  *poFeature = NULL;
    std::unique_ptr<CADGeometry> spoCADGeometry(poCADLayer.getGeometry( nFID ) );
    
    if( GetLastErrorCode() != CADErrorCodes::SUCCESS )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                 "Failed to get geometry with ID = %d from layer \"%s\". Libopencad errorcode: %d",
                 (int)nFID, poCADLayer.getName().c_str(), GetLastErrorCode() );
        return NULL;
    }
    
    poFeature = new OGRFeature( poFeatureDefn );
    
    switch( spoCADGeometry->getType() )
    {
        case CADGeometry::POINT:
        {
            CADPoint3D * const poCADPoint = ( CADPoint3D* ) spoCADGeometry.get();
            CADVector stPositionVector = poCADPoint->getPosition();
            poFeature->SetGeometryDirectly( new OGRPoint( stPositionVector.getX(),
                                                         stPositionVector.getY(), stPositionVector.getZ() ) );
            
            return poFeature;
        }
        case CADGeometry::LINE:
        {
            CADLine * const poCADLine = ( CADLine* ) spoCADGeometry.get();
            OGRLineString *poLS = new OGRLineString();
            poLS->addPoint( poCADLine->getStart().getPosition().getX(),
                           poCADLine->getStart().getPosition().getY(),
                           poCADLine->getStart().getPosition().getZ() );
            poLS->addPoint( poCADLine->getEnd().getPosition().getX(),
                           poCADLine->getEnd().getPosition().getY(),
                           poCADLine->getEnd().getPosition().getZ() );
            poFeature->SetGeometryDirectly( poLS );
            
            return poFeature;
        }
        case CADGeometry::CIRCLE:
        {
            CADCircle * const poCADCircle = ( CADCircle* ) spoCADGeometry.get();
            OGRGeometry *poCircle = OGRGeometryFactory::approximateArcAngles(
                    poCADCircle->getPosition().getX(), poCADCircle->getPosition().getY(),
                    poCADCircle->getPosition().getZ(),
                    poCADCircle->getRadius(), poCADCircle->getRadius(), 0.0,
                    0.0, 360.0,
                    0.0 );
            poFeature->SetGeometryDirectly( poCircle );
            
            return poFeature;
        }
            
        default:
        {
            CPLError( CE_Failure, CPLE_NotSupported,
                     "Unhandled feature." );
            
            return NULL;
        }
    }
    
    return NULL;
}

OGRFeature *OGRCADLayer::GetNextFeature()
{
    if( (int)poCADLayer.getGeometryCount() <= nNextFID )
    {
        return NULL;
    }
    
	OGRFeature  *poFeature = NULL;
	std::unique_ptr<CADGeometry> spoCADGeometry(poCADLayer.getGeometry( nNextFID ) );

	if( GetLastErrorCode() != CADErrorCodes::SUCCESS )
	{
		CPLError( CE_Failure, CPLE_NotSupported,
                  "Failed to get geometry with ID = %d from layer \"%s\". Libopencad errorcode: %d", 
				  nNextFID, poCADLayer.getName().c_str(), GetLastErrorCode() );
		return NULL;
	}

	poFeature = new OGRFeature( poFeatureDefn );

	switch( spoCADGeometry->getType() )
	{
		case CADGeometry::POINT:
		{
			CADPoint3D * const poCADPoint = ( CADPoint3D* ) spoCADGeometry.get();
			CADVector stPositionVector = poCADPoint->getPosition();
			poFeature->SetGeometryDirectly( new OGRPoint( stPositionVector.getX(),
											stPositionVector.getY(), stPositionVector.getZ() ) );
											
			++nNextFID;
			return poFeature;
		}
		case CADGeometry::LINE:
		{
			CADLine * const poCADLine = ( CADLine* ) spoCADGeometry.get();
			OGRLineString *poLS = new OGRLineString();
			poLS->addPoint( poCADLine->getStart().getPosition().getX(),
							poCADLine->getStart().getPosition().getY(),
							poCADLine->getStart().getPosition().getZ() );
			poLS->addPoint( poCADLine->getEnd().getPosition().getX(),
							poCADLine->getEnd().getPosition().getY(),
							poCADLine->getEnd().getPosition().getZ() );
			poFeature->SetGeometryDirectly( poLS );

			++nNextFID;
			return poFeature;
		}
		case CADGeometry::CIRCLE:
		{
			CADCircle * const poCADCircle = ( CADCircle* ) spoCADGeometry.get();
			OGRGeometry *poCircle = OGRGeometryFactory::approximateArcAngles(
				poCADCircle->getPosition().getX(), poCADCircle->getPosition().getY(),
				poCADCircle->getPosition().getZ(), 
				poCADCircle->getRadius(), poCADCircle->getRadius(), 0.0, 
				0.0, 360.0, 
				0.0
			);
			poFeature->SetGeometryDirectly( poCircle );

			++nNextFID;
			return poFeature;
		}

		default:
		{
			CPLError( CE_Failure, CPLE_NotSupported,
                  "Unhandled feature." );
			++nNextFID;
			return NULL;
		}
	}

	++nNextFID;
	return NULL;
}