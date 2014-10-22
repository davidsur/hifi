//
//  LightEntityItem.cpp
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 12/4/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include <QDebug>

#include <ByteCountCoding.h>

#include "EntityTree.h"
#include "EntityTreeElement.h"
#include "LightEntityItem.h"


EntityItem* LightEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    return new LightEntityItem(entityID, properties);
}

// our non-pure virtual subclass for now...
LightEntityItem::LightEntityItem(const EntityItemID& entityItemID, const EntityItemProperties& properties) :
        EntityItem(entityItemID, properties) 
{ 
    _type = EntityTypes::Light;
    setProperties(properties, true);

    // a light is not collide-able so we make it's shape be a tiny sphere at origin
    _emptyShape.setTranslation(glm::vec3(0.0f,0.0f,0.0f));
    _emptyShape.setRadius(0.0f);
}

EntityItemProperties LightEntityItem::getProperties() const {
    EntityItemProperties properties = EntityItem::getProperties(); // get the properties from our base class

    properties.setColor(getXColor());
    properties.setGlowLevel(getGlowLevel());
    properties.setIsSpotlight(getIsSpotlight());

    return properties;
}

bool LightEntityItem::setProperties(const EntityItemProperties& properties, bool forceCopy) {
    bool somethingChanged = EntityItem::setProperties(properties, forceCopy); // set the properties in our base class

    SET_ENTITY_PROPERTY_FROM_PROPERTIES(color, setColor);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(isSpotlight, setIsSpotlight);

    if (somethingChanged) {
        bool wantDebug = false;
        if (wantDebug) {
            uint64_t now = usecTimestampNow();
            int elapsed = now - getLastEdited();
            qDebug() << "LightEntityItem::setProperties() AFTER update... edited AGO=" << elapsed <<
                    "now=" << now << " getLastEdited()=" << getLastEdited();
        }
        setLastEdited(properties.getLastEdited());
    }
    return somethingChanged;
}

int LightEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead, 
                                                ReadBitstreamToTreeParams& args,
                                                EntityPropertyFlags& propertyFlags, bool overwriteLocalData) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY_COLOR(PROP_COLOR, _color);
    READ_ENTITY_PROPERTY(PROP_IS_SPOTLIGHT, bool, _isSpotlight);

    return bytesRead;
}


// TODO: eventually only include properties changed since the params.lastViewFrustumSent time
EntityPropertyFlags LightEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);
    requestedProperties += PROP_COLOR;
    requestedProperties += PROP_IS_SPOTLIGHT;
    return requestedProperties;
}

void LightEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params, 
                                    EntityTreeElementExtraEncodeData* modelTreeElementExtraEncodeData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount, 
                                    OctreeElement::AppendState& appendState) const { 

    bool successPropertyFits = true;
    APPEND_ENTITY_PROPERTY(PROP_COLOR, appendColor, getColor());
    APPEND_ENTITY_PROPERTY(PROP_IS_SPOTLIGHT, appendValue, getIsSpotlight());
}
