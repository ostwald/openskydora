<?php
/**
 * @file
 * Islandora solr search collection info template - shows collection description
 * and "search this collection" search box for collecions that have items (as
 * opposed to collections that have subcollections).
 *
 * Variables available:
 * - $variables: all array elements of $variables can be used as a variable.
 *   e.g. $base_url equals $variables['base_url']
 *
 *  - collection_pid
 *  - collection_title
 *  - collection_description
 *
 * @see template_preprocess_islandora_collection_info()
 */
?>

<div id="openskydora-search-collection-info">
<!-- <div class="debug-label">openskydora-search-collection-info</div> -->

<?php
    /* admin is 3, interns is 4 */
    if (user_has_role(3) || user_has_role(4)): ?>
    <div class="tabs-wrapper" style="margin:0px;padding-top:1px;">
      <ul class="tabs primary">
        <li><a href="#overlay=islandora/object/<?php print $variables['collection_pid']; ?>/manage">Manage</a></li>
      </ul>
    </div>
  <?php endif; ?>

    <h1 class="title" style="text-decoration:underline;">
        <?php print $variables['collection_title']; ?>
    </h1>

    <div id="collection-search">
      <h2 style="font-size:1.0em;">Search this collection</h2>
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_search_collection_search');
        print render($block['content']);
      ?>
    </div>

    <?php if (!empty($variables['collection_description'])): ?>
      <p><?php print $variables['collection_description']; ?></p>
    <?php endif; ?>
</div>
