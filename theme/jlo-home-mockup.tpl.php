<?php // dsm ($image_carousel_pids); ?>

<div id="block-storage" style="display:none">
  
  <div id="trending-items" class="trending-wrapper">
    <?php
      $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_items');
      print render($block['content']);
      ?>
  </div>
  <div id="trending-images" class="trending-wrapper">
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_images');
        print render($block['content']);
      ?>
  </div>
  <div id="trending-downloads" class="trending-wrapper">
    <?php
      $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_downloads');
      print render($block['content']);
      ?>
  </div>
  <div id="new-archives-items" class="trending-wrapper">
    <?php
      $block = module_invoke('openskydora', 'block_view', 'openskydora_new_archival_items');
      print render($block['content']);
      ?>
  </div>
  <div id="trending-collections" class="trending-wrapper">
    <?php
      $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_collections');
	  print render($block['content']);
      ?>
  </div>
  <div id="top-searched" class="trending-wrapper">
	<h3 style="color:orange">Top Searched</h3>
	<ul>
	  <?php foreach($top_searched as $item): ?>
	  <li><?php print $item->term; ?>
		<?php if (@$show_usage_data_count): ?>
		(<?php print $item['count']; ?>)
		<?php endif; ?>
	  </li>
	  <?php endforeach; ?>
	</ul>
  </div>
  <div id="image-carousel" class="trending-wrapper">
	<h3 style="color:orange">Trending Images</h3>
	<div id="carousel-image"></div>
  </div>
</div>


<div style="font-size:.85em;font-style:italic;">
  Usage stats computed over the past <?php print OPENSKYDORA_USAGE_WEEKS_AGO ?> weeks
</div>

<table class="usage-stats-table">
  <tr>
	<td id="cell-0" />
    <td id="cell-1"/>
	<td id="cell-2"/>
  </tr>
</table>


<img width="1000px" src="/<?php print $img_dir; ?>/mockup-bottom.png" /> 

<script language="javascript">
  /* initialize pids for the image carousel from php variable */
  var IMAGE_PIDS = [];
  <?php foreach($image_carousel_pids as $image_pid): ?>
  IMAGE_PIDS.push('<?php print $image_pid; ?>');
  <?php endforeach; ?>

  log (IMAGE_PIDS);
  
</script>

