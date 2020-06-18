
<?php if ($parent_collections): ?>
  <div>
    <h2><?php print t('In collections'); ?></h2>
    <ul>
      <?php foreach ($parent_collections as $collection): ?>
      <li><?php print l($collection->label, "islandora/object/{$collection->id}", array (
          'attributes' => array(
              'class' => 'in-collection-link'
          ),
      )); ?></li>
      <?php endforeach; ?>
    </ul>
  </div>
<?php endif; ?>
