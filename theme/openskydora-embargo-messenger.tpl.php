<?php

// dpm($variables);
?>
<!--
<div>hello <span style="font-family:arial;font-weight:bold"><?php print (@$variables['object']->id) ?></span>
    from openskydora-embargo-message template</div>

    <p>My parent is <span style="font-family:arial;font-weight:bold"><?php print (implode($parent_collections, ' & ')) ?></span>
-->
    
<?php if (in_array('islandora:jonscollection', $parent_collections)): ?>
    <h1 style="color:red;">jonscollection message title</h1>
<?php else: ?>
    <h1 style="color:red;">DEFFAULT collection messsage title</h1>

<?php endif; ?>
