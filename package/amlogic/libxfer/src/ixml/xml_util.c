/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: XML utility functions for UPnP.
 *
 *  Author: Eric Knudstrup
 *  Created: Wed Jul 27 14:12:59 2005
 *
 *******************************************************************/


#include <sys/types.h>
#include <stddef.h>

#include <ixml.h>
#include "xml_util.h"
#ifdef _MSC_VER
#include <string.h>
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#endif

/******************************************************************************/
DOMString get_tag_string ( IXML_Node * node, 
                           char * tag )
{
    IXML_Node * temp = get_child_tag(node, tag);

    if ( temp ) {
        IXML_Node * text = ixmlNode_getFirstChild(temp);

        if ( text ) {
            return ixmlNode_getNodeValue(text);
        }
    }

    return NULL;
}

/******************************************************************************/
IXML_Node * get_child_tag ( IXML_Node * node, 
                            char * tag )
{
    IXML_Node * temp;

    for( temp = ixmlNode_getFirstChild(node);
         temp;
         temp = ixmlNode_getNextSibling(temp) ) {
        if ( strcasecmp(temp->localName, tag) == 0 ) {
            return temp;
        }
    }

    return NULL;
}

/******************************************************************************/
DOMString get_attribute_string ( IXML_Node * node, 
                                 char * name )
{
    IXML_Node * temp = node->firstAttr;
    DOMString value = NULL;

    while ( temp ) {
        if ( strcasecmp(temp->nodeName, name) == 0 ) {
            value = ixmlNode_getNodeValue(temp);
            break;
        }
        else {
            temp = temp->nextSibling;
        }
    }

    return value;
}
