
// var COLLECTION_MANAGER = null;

(function ($) {

	/**
	   convenience for creating DOM elements
	*/
	function $t (tag) {
		tag = tag || 'div';
		return $('<'+tag+'/>');
	}    
	log ("reading collections");

	function safe_pid (pid) {
		return pid.replace(':', '_')
	}

	function real_pid (safe_pid) {
		return safe_pid.replace('_', ':')
	}

	var OPEN = "open";
	var CLOSED = "closed";

	// CONTROL WIDGET
	var CollectionControlWidget = Class.extend ({
		init: function (collection) {
			this.collection = collection;
			this.state = CLOSED;
			this.image = CLOSED_ICON;
			this.$dom = null;
//			this.set_state(OPEN);
			log ("controller for " + this.collection.pid);
			log ("   - " + this.collection.children.length + " children");
		},

		render: function (state) {
			state = (typeof state != 'undefined') ? null : state;

			if (state) {
				self.set_state(state)
			}
			this.$dom = $t('div')
				.addClass ('collection-control')
			if (this.collection.children.length > 0) {
				var self = this;
				this.$dom
					.html (this.image)
					.click (function (event) {
						self.toggle()
					})
			}
			return  this.$dom;
		},

		toggle: function () {
			log ('TOGGLE - my state is: ' + this.state);
			if (this.state == OPEN) {
				this.set_state (CLOSED)
			}
			else {
				this.set_state (OPEN)
			}
		},
		set_state: function(state) {
			log ("set_state - " + state);
			if (state == OPEN) {
				this.state = OPEN
				this.image = OPEN_ICON
				this.collection.open()
			} else if (state == CLOSED) {
				this.state = CLOSED;
				this.image = CLOSED_ICON;
				this.collection.close();
			}

			// refresh image
			this.$dom.html (this.image)
		},
	});
	
	// COLECTION
	var Collection = Class.extend ({
		
		init: function (data) {
			this.data = data
			this.pid = this.data.pid;
			this.title = this.data.title;
			this.children = this.data.children
			this.dom = null;
			this.$body = null;
			this.controller = new CollectionControlWidget(this);

		},
		report: function (data) {
			return "collection " + this.data.title;
		},

		close: function () {
			this.$body.slideUp();
		},

		open: function () {
			this.$body.slideDown();
		},
		
		toggle: function () {

			if (this.$body.is(":visible"))
				this.close();
			else
				this.open();
		},
			
		render: function () {
			var $thumbnail =$t('img')
				.addClass("thumbnail")
				.attr('src', this.data.object_url + '/datastream/TN');
			
			var $link = $t('a')
				//.attr('href','/jlo/coll_item?pid=' + this.pid)
				.attr('href', this.data.object_url)
				.append($thumbnail);

			var $collection_image = $t('div')
				.addClass("image")
				.html($link)				
/*
			if (this.children.length > 0) {


					.click (function (event) {  // thumbnail
						event.preventDefault()
						var id = $(event.target).closest('li').attr('id');
						log ("toggle my body: " + id);
						collection = COLLECTION_MANAGER.get_collection(real_pid(id));
						//						collection.toggle();
						window.location = collection.data.object_url;
					})

			} else {
				$collection_image.html($thumbnail)
			}
			*/
			var $title = $t('h3')
				.addClass('title')
				.html(this.title)

			var $title_link = $t('a')
				.addClass('title-link')
				.attr('href', this.data.object_url) // OpenSky landing page
				.attr ('title', 'Collection landing page on OpenSky')

				.html($title)

			
			var $description = $t('div')
				.addClass('description')
				.css("display", "none")
				.html(this.data.description.trimToLength(200))
			
			var $pid_link = $t('a')
				.addClass('pid-link')
				.attr('href','/jlo/coll_item?pid=' + this.pid) // INTERNAL
				.attr ('title', 'Debuging view')
				.html(this.pid)

			$toggle_widget = this.controller.render();
			
			var $head = $t('div')
				.addClass("collection-head")
				.append($toggle_widget)
				.append($collection_image)
				.append($pid_link)
				.append($title_link)
				.append($description)
			
			this.$body = $t('div')
				.css('display', 'none')
				.addClass('collection-body')

			if (this.children.length > 0) {
				
				var $children = $t('ul').addClass("collection-list");
				$(this.children).each (function (i, child_pid) {
					var child_collection = COLLECTION_MANAGER.get_collection (child_pid);
					$children.append ($t('li')
									  .attr ('id', safe_pid(child_pid))
									  .html(child_collection.render()))
				});
				this.$body.append($children);
			}
			
			var container = $('<div>')
				.addClass('jlo-collection')
				.append($head)
				.append(this.$body)
			
			return container;
			
		}
		
	});
	
	var CollectionManager = Class.extend ({
		init: function (data) {
			this.data = data;
			this.collections = {}
			var pids = Object.keys(this.data);
			var self = this;
			$(pids).each (function (i, pid) {
				self.collections[pid] = new Collection (self.data[pid]);
			})
			log ("collections read");

			this.root = this.collections[ROOT_PID];
			log (' children: ' + this.root.children.length);

			log (" Collection_Manager initialized");	
		},
		get_collection:function (pid) {
			return this.collections[pid];
		},
		
		report: function () {
			log ("collection manager has " + Object.keys(this.data).length + " collections");
		},

		render_test: function () {
			log ("render_test");

			var $dom = $('#islandora_root');
			    
//			slog(this.root);
			var self = this;
			$(this.root.children).each (function (i, child_pid) {
				log ("child_pid: " + i + ": " + child_pid);
				var collection = self.collections[child_pid]
				//$dom.append ($t('div').html (collection.title))

				$dom.append ($t('li')
							 .attr ('id', safe_pid(child_pid))
							 .append (collection.render()))
			});
		}
	});

	function render_collections () {
	
		$.ajax ({
			'url': '/jlo/get/collections',
			'success': function (response) {
				COLLECTION_MANAGER = new CollectionManager(response.data);
				COLLECTION_MANAGER.report();
				COLLECTION_MANAGER.render_test();
				log ('render is complete');
				
				var	node_selector = '.jlo-collection';
				node_selector = ".collection-head"
				$(node_selector)
					.mouseover (function (event) {
						
						$node = $(event.target).closest('li');
						$description = $node.find('.description').first();
						$description.show();
					})
					.mouseout (function (event) {
						$node = $(event.target).closest('li');
						$description = $node.find('.description').first();
						$description.hide();
					})
				
			}
		})
	}

//	render_collections();
	
}(jQuery));


