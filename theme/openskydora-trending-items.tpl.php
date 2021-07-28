<?php

//dsm($variables);

?>

<h3><?php print $title; ?></h3>

<ul>
  <?php foreach ($trending_items as $pid=>$item): ?>
    <li><?php print l($item['label'], "islandora/object/{$pid}"); ?>
 (<?php print $item['count']; ?>)
	</li>
  <?php endforeach; ?>
</ul>
