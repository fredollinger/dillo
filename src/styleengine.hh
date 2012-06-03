#ifndef __STYLEENGINE_HH__
#define __STYLEENGINE_HH__

class StyleEngine;

#include "dw/core.hh"
#include "doctree.hh"
#include "css.hh"
#include "cssparser.hh"

/**
 * \brief This class provides the glue between HTML parser and CSS subsystem.
 *
 * It maintains a document tree and creates and caches style objects for use
 * by the HTML parser.
 * The HTML parser in turn informs StyleEngine about opened or closed
 * HTML elements and their attributes via the startElement() / endElement()
 * methods.
 */
class StyleEngine {
   private:
      struct Node {
         CssPropertyList *styleAttrProperties;
         CssPropertyList *styleAttrPropertiesImportant;
         CssPropertyList *nonCssProperties;
         dw::core::style::Style *style;
         dw::core::style::Style *wordStyle;
         dw::core::style::Style *backgroundStyle;
         bool inheritBackgroundColor;
         DoctreeNode *doctreeNode;
      };

      dw::core::Layout *layout;
      lout::misc::SimpleVector <Node> *stack;
      CssContext *cssContext;
      Doctree *doctree;
      int importDepth;

      dw::core::style::Style *style0 (int i);
      dw::core::style::Style *wordStyle0 ();
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                CssPropertyValue value) {
         Node *n = stack->getRef (stack->size () - 1);

         if (!n->nonCssProperties)
            n->nonCssProperties = new CssPropertyList (true);
         n->nonCssProperties->set(name, type, value);
      }
      void preprocessAttrs (dw::core::style::StyleAttrs *attrs);
      void postprocessAttrs (dw::core::style::StyleAttrs *attrs);
      void apply (int i, dw::core::style::StyleAttrs *attrs, CssPropertyList *props);
      bool computeValue (int *dest, CssLength value,
                         dw::core::style::Font *font);
      bool computeValue (int *dest, CssLength value,
                         dw::core::style::Font *font, int percentageBase);
      bool computeLength (dw::core::style::Length *dest, CssLength value,
                          dw::core::style::Font *font);
      void computeBorderWidth (int *dest, CssProperty *p,
                               dw::core::style::Font *font);

   public:
      StyleEngine (dw::core::Layout *layout);
      ~StyleEngine ();

      void parse (DilloHtml *html, DilloUrl *url, const char *buf, int buflen,
                  CssOrigin origin);
      void startElement (int tag);
      void startElement (const char *tagname);
      void setId (const char *id);
      const char * getId () { return doctree->top ()->id; };
      void setClass (const char *klass);
      void setStyle (const char *style);
      void endElement (int tag);
      void setPseudoLink ();
      void setPseudoVisited ();
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                int value) {
         CssPropertyValue v;
         v.intVal = value;
         setNonCssHint (name, type, v);
      }
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                const char *value) {
         CssPropertyValue v;
         v.strVal = dStrdup(value);
         setNonCssHint (name, type, v);
      }
      void inheritNonCssHints ();
      void clearNonCssHints ();
      void restyle ();
      void inheritBackgroundColor (); /* \todo get rid of this somehow */
      dw::core::style::Style *backgroundStyle ();
      dw::core::style::Color *backgroundColor ();

      inline dw::core::style::Style *style () {
         dw::core::style::Style *s = stack->getRef (stack->size () - 1)->style;
         if (s)
            return s;
         else
            return style0 (stack->size () - 1);
      };

      inline dw::core::style::Style *wordStyle () {
         dw::core::style::Style *s = stack->getRef(stack->size()-1)->wordStyle;
         if (s)
            return s;
         else
            return wordStyle0 ();
      };
};

#endif
