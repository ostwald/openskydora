<?php

/**
 * @file
 * islandora-basic-collection-wrapper.tpl.php
 *
 * @TODO: needs documentation about file and variables
 */
?>

<div class="islandora-basic-collection-wrapper">
  <!--  <div class="debug-label">openskydora-basic-collection-wrapper.tpl.php</div> -->

  <?php if ($islandora_object->id != 'opensky:root'): ?>
    <div id="collection-search">
      <h2 style="font-size:1.0em">Search this collection</h2>
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_browse_collection');
        print render($block['content']);
      ?>
    </div>
  <?php endif; ?>

  <?php if (!$display_metadata && !empty($dc_array['dc:description']['value'])): ?>
    <p><?php print nl2br($dc_array['dc:description']['value']); ?></p>
  <?php endif; ?>
  
  <div class="islandora-basic-collection clearfix">
    <?php if ($islandora_object->id != 'opensky:root'): ?>
      <hr />
    <?php endif; ?>

    <!-- end OpenSky customization -->

    <span class="islandora-basic-collection-display-switch">
      <ul class="links inline">
        <?php foreach ($view_links as $link): ?>
          <li>
            <a <?php print drupal_attributes($link['attributes']) ?>><?php print filter_xss($link['title']) ?></a>
          </li>
        <?php endforeach ?>
      </ul>
    </span>
    <?php print $collection_pager; ?>
    <?php print $collection_content; ?>
    <?php print $collection_pager; ?>
  </div>
  <?php if ($display_metadata): ?>
    <div class="islandora-collection-metadata">
      <?php print $description; ?>
      <?php if ($parent_collections): ?>
        <div>
          <h2><?php print t('In collections'); ?></h2>
          <ul>
            <?php foreach ($parent_collections as $collection): ?>
              <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
            <?php endforeach; ?>
          </ul>
        </div>
      <?php endif; ?>
      <?php print $metadata; ?>
    </div>
  <?php endif; ?>
</div>
