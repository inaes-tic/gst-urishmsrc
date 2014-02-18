/*
 * GstURIShmsrc
 *
 * Copyright © <2014> Instituto Nacional de Asociativismo y Economia Social.
 * Copyright © <2014> Cooperativa de Trabajo OpCode Limitada <info@opcode.coop>.
 * Copyright © <2014> Adrián Pardini     <adrian@opcode.coop>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * SECTION:element-urishmsrc
 *
 * FIXME:Describe urishmsrc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! urishmsrc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>

#include "gsturishmsrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_urishmsrc_debug);
#define GST_CAT_DEFAULT gst_urishmsrc_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_URI
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src_%u",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

static void gst_urishmsrc_src_handler_init (gpointer g_iface,
    gpointer iface_data);
static GstURIType gst_urishmsrc_src_get_uri_type (GType type);
static const gchar *const *gst_urishmsrc_src_get_protocols (GType type);
static gchar *gst_urishmsrc_src_get_uri (GstURIHandler * handler);
static gboolean gst_urishmsrc_src_set_uri (GstURIHandler * handler,
    const gchar * uri, GError ** error);


#define gst_urishmsrc_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstURIShmsrc, gst_urishmsrc, GST_TYPE_BIN,
    G_IMPLEMENT_INTERFACE (GST_TYPE_URI_HANDLER,
        gst_urishmsrc_src_handler_init));

static void gst_urishmsrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_urishmsrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void
gst_urishmsrc_src_handler_init (gpointer g_iface, gpointer iface_data)
{
  GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

  iface->get_type = gst_urishmsrc_src_get_uri_type;
  iface->get_protocols = gst_urishmsrc_src_get_protocols;
  iface->get_uri = gst_urishmsrc_src_get_uri;
  iface->set_uri = gst_urishmsrc_src_set_uri;
}

/* GObject vmethod implementations */
static GstURIType
gst_urishmsrc_src_get_uri_type (GType type)
{
  return GST_URI_SRC;
}

static const gchar *const *
gst_urishmsrc_src_get_protocols (GType type)
{
  static const gchar *protocols[] = { "shm", 0 };

  return protocols;
}

static gchar *
gst_urishmsrc_src_get_uri (GstURIHandler * handler)
{
  GstURIShmsrc *self = GST_URISHMSRC (handler);

  return g_strdup (self->uri);
}

static gboolean
gst_urishmsrc_src_set_uri (GstURIHandler * handler, const gchar * uri,
    GError ** error)
{
  GstURIShmsrc *self = GST_URISHMSRC (handler);
  gboolean ret = FALSE;
  const gchar *orig_uri = uri;

  GST_OBJECT_LOCK (self);
  if (GST_STATE (self) >= GST_STATE_PAUSED) {
    goto wrong_state;
  }
  GST_OBJECT_UNLOCK (self);

  if (strncmp ("shm:", uri, 4) != 0) {
    goto invalid_uri;
  }

  uri += 6;
  self->uri = g_strdup (orig_uri);
  ret = TRUE;

out:
  return ret;

wrong_state:
  {
    GST_WARNING_OBJECT (self, "Can't set URI in %s state",
        gst_element_state_get_name (GST_STATE (self)));
    GST_OBJECT_UNLOCK (self);
    g_set_error (error, GST_URI_ERROR, GST_URI_ERROR_BAD_STATE,
        "Changing the 'uri' property on urishmsrc while it is running "
        "is not supported");
    goto out;
  }

invalid_uri:
  {
    GST_WARNING_OBJECT (self, "invalid URI '%s'", uri);
    g_set_error (error, GST_URI_ERROR, GST_URI_ERROR_BAD_URI,
        "Invalid data URI");
    goto out;
  }
}



/* initialize the urishmsrc's class */
static void
gst_urishmsrc_class_init (GstURIShmsrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_urishmsrc_set_property;
  gobject_class->get_property = gst_urishmsrc_get_property;

  g_object_class_install_property (gobject_class, PROP_URI,
      g_param_spec_string ("uri",
          "URI",
          "URI that should be used",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  gst_element_class_set_static_metadata (gstelement_class,
      "Shared memory source addressable with an uri.",
      "Source",
      "Reads audio and video from shared memory using a uri.",
      "Adrián Pardini <adrian@opcode.coop>, "
      "Cooperativa de Trabajo OpCode Limitada <info@opcode.coop>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_urishmsrc_init (GstURIShmsrc * self)
{


}

static void
gst_urishmsrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstURIShmsrc *self = GST_URISHMSRC (object);

  switch (prop_id) {
    case PROP_URI:
      gst_urishmsrc_src_set_uri (GST_URI_HANDLER (self),
          g_value_get_string (value), NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_urishmsrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstURIShmsrc *self = GST_URISHMSRC (object);

  switch (prop_id) {
    case PROP_URI:
      g_value_set_string (value, self->uri);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */



/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
urishmsrc_init (GstPlugin * urishmsrc)
{
  /* debug category for fltering log messages
   *
   */
  GST_DEBUG_CATEGORY_INIT (gst_urishmsrc_debug, "urishmsrc", 0, "urishmsrc");

  return gst_element_register (urishmsrc, "urishmsrc", GST_RANK_PRIMARY,
      GST_TYPE_URISHMSRC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gsturishmsrc"
#endif

/* gstreamer looks for this structure to register elements.
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    urishmsrc,
    "urishmsrc",
    urishmsrc_init,
    VERSION,
    "LGPL",
    "gsturishmsrc",
    "http://opcode.coop"
)
