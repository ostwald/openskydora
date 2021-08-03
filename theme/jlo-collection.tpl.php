<?php
/* Requires jlo_collection */
  // dsm($jlo_collection);

  $collection_has_children = (count($jlo_collection['children']) > 0);
?>

<li id="<?php print str_replace(':','_',$jlo_collection['pid']) ?>">
  <?php $toggle_class = $collection_has_children ? 'toggler' : ''; ?>
  <div class="jlo-collection <?php print $toggle_class?>">
	<div class="collection-head">
	  <?php if ($collection_has_children): ?>
	  <div class="collection-control">
		<?php print $closed_icon; ?>
	  </div>
	  <?php else: ?>
	  <div class="collection-control">
	  </div>
	  <?php endif; ?>

	  <div class="image">
		<img class="thumbnail" src="<?php print
										  $jlo_collection['object_url'];?>/datastream/TN"/>
	  </div>

	  <a class="pid-link" href="/jlo/coll_item?pid=<?php print$jlo_collection['pid']; ?>"
		 title="Debugging View"><?php print$jlo_collection['pid']; ?></a>

	  <h3 class="title"><?php print $jlo_collection['title']; ?></h3>

	  <div class="description" style="display:block"><?php print $jlo_collection['description']; ?></div>
	</div>
	<div class="collection-body" style="display:none">
	  <?php if (count($jlo_collection['children']) > 0): ?>
	  <ul class="collection-list">
		<?php foreach ($jlo_collection['children'] as $child): ?>
		  <?php print render_collection ($child); ?>
		<?php endforeach; ?>
	  </ul>
	  <?php endif; ?>
	</div>
  </div>
</li>



