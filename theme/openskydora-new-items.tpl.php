<?php

//dsm($variables);

?>

<h3><?php print $title; ?></h3>

<ul>
  <?php foreach ($new_items as $item): ?>
    <li><?php print l($item['label'], "islandora/object/{$item['pid']}"); ?>
	  <?php if (@$show_usage_data_date): ?>
	    (<?php print $item['created']; ?>)
	  <?php endif; ?>
	</li>
  <?php endforeach; ?>
</ul>
