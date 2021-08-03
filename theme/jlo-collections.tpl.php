<?php
// dsm ($variables);

?>

<script>

var ROOT_PID = '<?php print $root_pid; ?>';
var OPEN_ICON = '<?php print $open_icon; ?>';
var CLOSED_ICON = '<?php print $closed_icon; ?>';


window.addEventListener( "pageshow", function ( event ) {
  var historyTraversal = event.persisted || 
                         ( typeof window.performance != "undefined" && 
                              window.performance.navigation.type === 2 );
	log ("pageShow: " + window.document.referrer);
	if ( historyTraversal ) {
		// Handle page restore.
		
		if (typeof (window.decorate_collections) != 'undefined') {
			// log ('NOT redecorating ....')
			//log ('DECOPRATE COLLECTIONS Accessible')			
			window.decorate_collections()
		}
		else {
			log ("reloading ...");
			window.location.reload(true);
		}
	}
});


(function ($) {    
    // jQuery is ready ...
   
}(jQuery));

</script>

<ul id="islandora_root" class="collection-list">
  <?php foreach ($jlo_collections[$root_pid]['children'] as $child): ?>
     <?php print render_collection ($child); ?>
  <?php endforeach; ?>

</ul>



