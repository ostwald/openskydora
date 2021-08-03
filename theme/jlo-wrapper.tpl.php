<style>
  h3 {
  margin:20px 0 10px;
  }

  .jlo_error {
  color:red;
  font-size:0.70em;
  }
  </style>
<div id="wrapper-description">
<!-- This is the jlo-wrapper template -->
</div>

     
<h2>Usage Stats</h2>

     <h3>Top Searches</h3>
	 <table>
	   <tr>
		 <th>term</th>
		 <th>searches</th>
	   </tr>
	   
	   <?php foreach ($top_searches as $search): ?>
       <tr>
		 <td>
		   <?php print $search->term; ?>
		 </td>
		 <td>
		   <?php print $search->search_total; ?>
		 </td>
	 </tr>
     <?php endforeach; ?>
	 </table>
	 
<h3>Most viewed collections in past
<?php print $most_viewed_collections_weeks_ago; ?> weeks</h3>
<?php // dsm ($most_viewed_collections); ?>
<table>
  <tr>
	<th class="pid_col">pid</th>
	<th class="label-col">label</th>
	<th class="download-col">views</th>
  </tr>
<?php foreach ($most_viewed_collections as $coll_pid=>$views): ?>
    <tr>
	  <td>
		<?php print $coll_pid; ?>
	  </td>
	  <td>
		<?php $obj = islandora_object_load(str_replace('info:fedora/','',
			  $coll_pid));
			  if ($obj) {
			    print l($obj->label, "islandora/object/{$coll_pid}");
		      } else {
		        print "Collection object not found";
		      }
		 ?>
	  </td>
	  <td>
	  <?php print $views; ?>
	  </td>
	</tr>
	<?php endforeach; ?>
</table>
     
<h3>Top Downloads in past <?php print $most_downloaded_weeks_ago; ?> weeks</h3>
<?php // dsm($most_downloaded); ?>

<table>
  <tr>
	<th class="pid_col">pid</th>
	<th class="label-col">label</th>
	<th class="download-col">downloads</th>
  </tr>
  <?php foreach ($most_downloaded as $item): ?>
    <tr>
	  <td>
		<?php print $item->pid; ?>
	  </td>
	  <td>
		<?php print l($item->label, "islandora/object/{$pid}") ?>
	  </td>
	  <td>
	  <?php print $item->downloads; ?>
	  </td>
	</tr>
	<?php endforeach; ?>
</table>


<h3>Most Viewed Items in past <?php print
    $most_viewed_items_weeks_ago ; ?> weeks</h3>
    <?php // dsm($most_viewed_items); ?>
<table>
  <tr>
	<th class="pid_col">pid</th>
	<th class="label-col">label</th>
	<th class="view-col">views</th>
  </tr>
  <?php foreach ($most_viewed_items as $pid=>$item): ?>
    <tr>
	  <td>
		<?php print $pid; ?>
	  </td>
	  <td>
		<?php print l($item->obj->label, "islandora/object/{$pid}"); ?>
	  </td>
	  <td>
	  <?php print $item->views; ?>
	  </td>
	</tr>
	<?php endforeach; ?>
</table>

<script>

function log (s) {
    if (window.console)
        console.log (s);
}
  
(function ($) {

// DOM is ready and so is jquery
log ("hello from the template! ");

}(jQuery));

</script>
