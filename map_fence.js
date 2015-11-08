// Source: [DEPRECATED]
// Google Maps JavaScript API v2
// https://groups.google.com/forum/#!forum/google-maps-api

// geofence hints

// http://gmaps-samples.googlecode.com/svn/trunk/poly/mymapstoolbar.html

function startDrawing(poly, name, onUpdate, color) {
        map.addOverlay(poly);
        poly.enableDrawing(options);
        //poly.enableEditing({onEvent: "mouseover"});
        poly.disableEditing({onEvent: "mouseout"});
        GEvent.addListener(poly, "endline", function() {
          select("hand_b");
          alert("on click");
          var cells = addFeatureEntry(name, color);
          GEvent.bind(poly, "lineupdated", cells.desc, onUpdate);
          
        });
      }

// some more for further look...

// http://code.google.com/articles/support/ezdigitizer.htm

// http://maps.forum.nu/gm_drag_polygon.html

// http://code.google.com/apis/maps/documentation/javascript/v2/controls.html#Custom_Controls 

// http://www.google.com/search?q=javascript+button+trigger+javascript 

// http://code.google.com/apis/maps/articles/phpsqlinfo.html 
