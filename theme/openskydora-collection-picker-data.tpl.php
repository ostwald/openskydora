<?php
/**
 * Created by IntelliJ IDEA.
 * User: ostwald
 * Date: 11/30/17
 * Time: 1:59 PM
 */

//dsm('theme/opensky-collection-picker.tpl.php');
//dsm($_GET);

?>

<script>

    var COLLECTION_SELECT_OPTIONS = {};
    <?php foreach ($option_data as $key => $value): ?>
        COLLECTION_SELECT_OPTIONS["<?php print $key ?>"] = "<?php print $value ?>";
    <?php endforeach ?>

</script>

<script>
// Below is orgNav code that used to be inserted as a separate block but that
// stopped working around 11/13 for unknown reasons ...

// CUSTOM VARIABLES
var contactLink = "mailto:ogg@ucar.edu";     // enter your Webmaster/Feedback
                                             // link. Include http:// or mailto:
var hideOrgNav = false;        // hide the entire OrgNav from view
var hideFooter = false;          // hide the entire OrgFooter from view
var hideNSF = true;               // hide the NSF disclaimer
var footerColor = '#1B477C';       // hex color of the footer font including the
                                   // #

// DO NOT EDIT BELOW THIS POINT
var jsHost = (("https:" == document.location.protocol) ? "https://" : "http://");
if(typeof jQuery === "undefined"){
    document.write("<scr"+"ipt src='"+jsHost+"ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js' type='text/javascript'></scr"+"ipt>");
}
document.write("<scr"+"ipt src='"+jsHost+"orgnav.ucar.edu/orgnav.js' type='text/javascript'></scr"+"ipt>");
</script>
<noscript><iframe frameborder="0" width="100%" src="https://orgnav.ucar.edu"></iframe></noscript>

