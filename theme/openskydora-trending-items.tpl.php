<?php

// dsm($variables);

?>

<h3><?php print $title; ?></h3>

<ul>
  <?php foreach ($trending_items as $pid=>$item): ?>
    <li><?php print l($item['label'], "islandora/object/{$pid}"); ?>
	  <?php if (@$show_usage_data_count): ?>
	    (<?php print $item['count']; ?>)
	  <?php endif; ?>
	</li>
  <?php endforeach; ?>
</ul>
