/*
  Copyright (C) 2011 David Robillard
  Copyright (C) 2013 Paul Davis
    
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or (at
  your option) any later version.
    
  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.
    
  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __jack_metadata_h__
#define __jack_metadata_h__

#include <jack/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Metadata Metadata API.
 * @{
 */

/**
 * A single property (key:value pair).
 */
typedef struct {
    const char* key;   /**< The key of this property (URI string). */
    const char* data;  /**< The property value (null-terminated string) */
    const char* type;  /**< MIME type of data. Likely values are:
                        *
                        *  text/utf8  (for an null terminated string)
                        *  image/png;base64 (for a data-URI converted image)
                        *
                        * If type is null (or empty), the type should
                        * be assumed to be "text/utf8" and the memory
                        * pointed to by "data" should be interpreted
                        * as a null-terminated string encoded using UTF-8.
                        *
                        * If the type is image/png;base64, the memory 
                        * pointed to by "data" should be interpreted as 
                        * a base64 encoded PNG image.
                        *
                        * Other types are subject to the shared understanding
                        * of the mime type by both the setter and retriever
                        * of the property.
                        */
} jack_property_t;

        
/**
 * Set a property on @c subject.
 *
 * See the above documentation for rules about @c subject and @c key.
 * @param subject The subject to set the property on.
 * @param key The key of the property.
 * @param value The value of the property.
 * @param type The MIME type of the property. See the discussion of
 *              types in the definition of jack_property_t above.
 * @return 0 on success.
 */
int
jack_set_property(jack_client_t*,
                  jack_uuid_t subject,
                  const char* key,
                  const char* value,
                  const char* type);

/**
 * Get a property on @c subject.
 *
 * @param subject The subject to get the property from.
 * @param key The key of the property.
 * @param value Set to the value of the property if found, or NULL otherwise.
 *              The caller must free this value with jack_free().
 * @param type The MIME type of the property if set, or
 *              NULL. See the discussion of types in the definition of
 *              jack_property_t above.  If non-null, the caller must free
 *              this value with jack_free().
 *
 * @return 0 on success, -1 if the @c subject has no @c key property.
 */
int
jack_get_property(jack_uuid_t subject,
                  const char* key,
                  char**      value,
                  char**      type);

/**
 * A description of a subject (a set of properties).
 */
typedef struct {
    jack_uuid_t      subject;     /**< The subject being described. */
    uint32_t         property_cnt;/**< The number of properties stored in "properties" */
    jack_property_t* properties;  /**< An array of properties. */
    uint32_t         property_size; /**< private, don't use or touch */
} jack_description_t;

/**
 * Free a description.
 *
 * @param desc a jack_description_t whose associated memory will all be released
 * @param free_description_itself if non-zero, then @param desc will also be passed to free()
 */
void
jack_free_description(jack_description_t* desc, int free_description_itself);

/**
 * Get a description of @c subject.
 * @param subject The subject to get all properties of.
 * @param desc Set to the description of subject if found, or NULL otherwise.
 *             The caller must free this value with jack_free_desription().
 * @return 0 on success, -1 if no @c subject with any properties exists.
 */
int
jack_get_properties (jack_uuid_t          subject,
                     jack_description_t*  desc);

/**
 * Get descriptions for all subjects with metadata.
 * @param subject The subject to get all properties of.
 * @param descs Set to a NULL-terminated array of descriptions.
 *              The caller must free each of these with jack_free_desription(),
 *              and the array itself with jack_free().
 * @return 0 on success.
 */
int
jack_get_all_properties (jack_description_t** descs);

/** Remove a single property on a subject
 *
 * @param client The JACK client making the request to remove the property.
 * @param subject The subject to remove the property from.
 * @param key The key of the property to be removed.
 *
 * @return 0 on success, -1 otherwise
 */
int jack_remove_property (jack_client_t*, jack_uuid_t subject, const char* key);

/** Remove all properties on a subject
 *
 * @param client The JACK client making the request to remove some properties.
 * @param subject The subject to remove all properties from.
 *
 * @return a count of the number of properties removed, or -1 if an error occurs
 */
int jack_remove_properties (jack_client_t*, jack_uuid_t subject);

/** Remove all properties
 *
 * WARNING!! This deletes all metadata managed by a running JACK server. 
 * Data lost cannot be recovered (though it can be recreated by new calls
 * to jack_set_property()). 
 * 
 * @param client The JACK client making the request to remove all properties
 *
 * @return 0 on success, -1 otherwise
 */
int jack_remove_all_properties (jack_client_t*);

typedef enum {
        PropertyCreated,
        PropertyChanged,
        PropertyDeleted
} jack_property_change_t;

typedef void (*JackPropertyChangeCallback)(jack_uuid_t subject,
                                           const char* key,
                                           jack_property_change_t change,
                                           void* arg);

/** Arrange for @param client to call @param callback whenever
 * a property is created, changed or deleted.
 *
 * @return 0 success, -1 otherwise.
 */
int jack_set_property_change_callback (jack_client_t* client,
                                       JackPropertyChangeCallback callback,
                                       void *arg);

#ifdef __cplusplus
} /* namespace */
#endif

/**
 * @}
 */

#endif  /* __jack_metadata_h__ */
