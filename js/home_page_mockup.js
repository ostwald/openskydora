
(function ($) {

	function store_block (block) {
		$('#block-storage').append($(block));
	}

	function load_block (dest, content) {
		$(dest)
			.append ($(content.show()))
			.show();
	}

	var ImageCarousel = Class.extend ({
		init: function (dom, image_pids) {
			this.$dom = $(dom);
			this.image_pids = image_pids;
			this.images = this.load_images();

			this.set_image(this.images[2]);
			var self = this;
			var i=1;

			if (1) {
				setInterval (function () {
					var x = i++ % self.images.length;
					//log ("x: " + x);
					var $container = self.$dom.find('#carousel-image');
					$container.fadeOut (1500, function () {
						self.set_image(self.images[x]);
						$container.fadeIn(1500);
					});
				}, 7000);
			}

			log ('ImageCarousel initiated');
		},

		set_image: function (image) {
			// this.$dom.find('img').attr('src', $(image).attr('src'));
			this.$dom.find('#carousel-image')
				.html(image);
		},
									   
		load_images: function () {
			var images=[];
			$(this.image_pids).each(function (i, pid) {
				//image = new Image(200,200);
				var image = new Image();
				image.src = '/islandora/object/'+pid+'/datastream/TN/view';
				log ('image ' + pid);
				var href = '/islandora/object/'+pid;
				var $link = $('<a/>')
					.attr('href', '/islandora/object/'+pid)
					.html(image);
				images.push($link);
//				images.push(image);
			});
			return images;
		}
	});
			
	
	$(function() {

		var $block_storage = $('#block-storage');
		$block_storage.children('.trending-wrapper').each(function (i, child) {
			var $child = $(child);
			$child.hide();
			if ($child.attr('id') == 'trending-collections') {
				$child.show();
			}
		});

		try {
			var carousel = new ImageCarousel ('#image-carousel', IMAGE_PIDS);
		} catch (error) {
			log ("error: " + error);
		}
		log ("hellothere");
		load_block ($('#cell-0'), $('#trending-collections'));
		load_block ($('#cell-1'), $('#trending-downloads'));
		load_block ($('#cell-2'), carousel.$dom);

	});


	
}(jQuery));

