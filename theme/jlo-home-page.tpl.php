<?php // dsm ($variables); ?>
<style>

#home-page-content div {
    margin-bottom:5px;
 }
#home-page-content div.heading {
    font-size:1.3em;
 }
#home-page-content div.spotlight-section {
  font-size:1.4em;
}

#home-page-content #home-items {
margin:auto;
width:70%;
}

#home-page-content .item {
/*  clear:all; 
overflow:auto; */

position:relative;
top:0px;
display:table-cell;
valign:top;
width:200px;
height:200px;
margin:5px;
padding:5px;
border:thin grey solid;
}

#home-page-content .item img {
/*	position:absolute; */
	height:200px;
	width:200px;
}

#home-page-content .item>div {
  text-align:center;
}

#home-menu a{
  color:#163a69;
}
#home-menu a:hover{
  text-decoration:underline;
}
  
#home-menu {
  text-align:center;
  width:auto;
  margin:auto;
  padding:15px;
}
#home-menu>div {
  display:inline-block;
  border:thin gray inset;
  text-align:center;
  padding:5px;
  font-size:1.4em;
  margin:auto 10px;
}

.trending-wrapper {
display:table-cell;
border:thin black solid;
padding:10px;
font-size:.85em;
width:250px;
valign:top;
}

.trending-wrapper h3 {
text-align:center;
}

</style>
<div id="home-page-content">
  <h2 align="center">Welcome to OpenSky!</h2>

      
  <div id="home-menu">
	<div><a href="/islandora/search/?type=dismax">Browse Items</a></div>
	<div><a href="/jlo/collections">Browse Collections</a></div>
  </div>

      <div class="trending-wrapper">
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_items');
        print render($block['content']);
      ?>
	  </div>
      <div class="trending-wrapper">
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_images');
        print render($block['content']);
      ?>
	  </div>
      <div class="trending-wrapper">
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_trending_downloads');
        print render($block['content']);
      ?>
	  </div>
      <div class="trending-wrapper">
      <?php
        $block = module_invoke('openskydora', 'block_view', 'openskydora_new_archival_items');
        print render($block['content']);
      ?>
      </div>

  <div id="home-items">
	<div class="item">
	  <img src="/<?php print($img_dir); ?>/TN.png"/>
	  <div class="heading">Home of the NCAR Technical Notes Collection</div>  
	</div>
	
	<!--  
		  <div class="item">
			<img src="/<?php print($img_dir); ?>/research.png"/>  
			<div class="heading">Please Browse our other 20-some Collections</div>
		  </div>
		  -->
	<div class="item">
	  <img src="/<?php print($img_dir); ?>/siparcs.png"/>  
	  <div class="spotlight-section">Spotlighted Collection: SIParCS</div>
	</div>
	
	<div class="item">
	  <img src="/<?php print($img_dir); ?>/archives.png"/>
	  <div class="spotlight-section">What&apos;s new in the Archives</div>
	</div>
	<br/>
	<div class="item">
	  <img src="/<?php print($img_dir); ?>/student_research.png"/>
	  <div class="spotlight-section">Recent Work from Students</div>
	</div>
	
	<div class="item">
      <img src="/<?php print($img_dir); ?>/dataset.png"/>
	  <div class="spotlight-section">Papers linked to Datasets</div>
	</div>
	
  </div>
</div>
<script>
	
	(function ($) {

	  $(function () {
		  $("#home-page-content .item")
			  .mouseover (function (event) {
				  var $item = $(event.target).closest('.item');
				  var $img = $item.find ('img');
				  $img.css ('opacity', 0.5);
			  })
			  .mouseout (function (event) {
				  var $item = $(event.target).closest('.item');
				  var $img = $item.find ('img');
				  $img.css ('opacity', 1.0);
			  })
	  });
  }(jQuery));
</script>
