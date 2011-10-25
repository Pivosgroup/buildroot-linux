/*******************************************************************
 * 
 * Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 * Description: XML utility functions for UPnP.
 *
 * Author: Eric Knudstrup
 * Created: Wed Jul 27 14:12:59 2005
 *
 *******************************************************************/


#ifndef XML_UTIL_H
#define XML_UTIL_H

DOMString get_tag_string ( IXML_Node * node, 
                           char * tag );

IXML_Node * get_child_tag ( IXML_Node * node, 
                            char * tag );

DOMString get_attribute_string ( IXML_Node * node, 
                                 char * name );

#endif /* XML_UTIL_H */
