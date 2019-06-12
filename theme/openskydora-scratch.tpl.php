<div>We want to build a data structure that will tell us the following given a collection PID:</div>
<ul class="scratch">
  <li>The collection name</li>
  <li>The subcollections (recursively or all together?)</li>
  <li><span style="color:gray">The number of direct children</span></li>
</ul>

     <div class="fooberry">Once we figure out how to get these answers, 
       we will write a routine to calculate the info for all collections and cache it
       so it doesn't have to be caluculated from scratch every thime
     </div>

<hr/>
<div>
  <h3><?php print ($title); ?> (<?php print $pid ?>)</h3>

  <div style="float:left;width:30%;padding:10px;">
    <h4>Searchable Subs (<?php print count($searchables); ?>)</h4>
    <ul class="scratch">
    <?php foreach($searchables as $sub_pid): ?>
      <li><a href="?pid=<?php print ($sub_pid); ?>"><?php print ($sub_pid); ?></a></li>
    <?php endforeach; ?>
    </ul>
  </div>


  <div style="float:left;width:30%;padding:10px;">
    <h4>Sub collections (<?php print count($children); ?>)</h4>
    <ul class="scratch">
    <?php foreach($children as $sub_pid): ?>
      <li><a href="?pid=<?php print ($sub_pid); ?>"><?php print ($sub_pid); ?></a></li>
    <?php endforeach; ?>
    </ul>
  </div>

  <div style="float:left;width:30%;padding:10px;">
    <h4>descendants (<?php print count($descendants); ?>)</h4>
    <ul class="scratch">
    <?php foreach($descendants as $sub_pid): ?>
      <li><a href="?pid=<?php print ($sub_pid); ?>"><?php print ($sub_pid); ?></a></li>
    <?php endforeach; ?>
    </ul>
  </div>
</div>
<br clear="all" />
<hr />
<ul class="scratch">
  <?php foreach($tree as $sub_pid => $items): ?>
    <?php if (openskydora_has_subcollections($sub_pid)): ?>
      <li><a href="?pid=<?php print $sub_pid; ?>"><?php print $sub_pid; ?></a>
      <ul class="scratch">
        <?php foreach($items as $item): ?> 
          <li><a href="?pid=<?php print $item; ?>"><?php print $item; ?></a></li>
        <?php endforeach; ?>
      </ul>
      </li>
    <?php endif; ?>
  <?php endforeach; ?>
</ul>