<?php

?>
<div id="jlo-checksum-results">
<hr/>
<h3>Checksum Results</h3>

<?php if ($scope == 'object'): ?>
    
   <p>PID = <?php  print $pid ?></p>
   <?php if (isset($error)): ?>
      <div class="error-msg"><?php print $error; ?></div>
   <?php endif; ?>
<?php endif; ?>
