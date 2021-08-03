/* 
   OPEN_ and CLOSED_ICON are defined in
*/
(function ($) {

	/**
	   convenience for creating DOM elements
	*/
	function $t (tag) {
		tag = tag || 'div';
		return $('<'+tag+'/>');
	}    

	function safe_pid (pid) {
		return pid.replace(':', '_')
	}

	function real_pid (safe_pid) {
		// dont change if alread a real pid
		if (safe_pid.indexOf(':') > -1) {
			return safe_pid;
		}
		return safe_pid.replace('_', ':');
	}

	var OPEN = "open";
	var CLOSED = "closed";
/*
	function collection_toggler (event) {
		var id = $(event.target).closest('li').attr('id');
		var collection = get_collection (id);
		collection.controller.toggle();
	}
*/
	function get_collection_for_el (el) {
		var $el = $(el);
		var id = $el.closest('li').attr('id');
		if (id) {
			return get_collection (id);
		}
	}

	function get_pid_for_object_url (url) {
		log ("get_pid from " + url);
		let re = new RegExp("/islandora/object/(.*)");
		var test = re.exec(url);
		if (test) {
			log ("TEST");
			return test[1];
		} else {
			log ('pid not found in ' + url);
		}
		return null;
	}
	
	function get_collection (id) {
		var pid = real_pid (id);
		return COLLECTION_MANAGER.get_collection(pid);
	}

	// CONTROL WIDGET
	var CollectionControlWidget = Class.extend ({
		init: function (collection) {
			// slog(collection)
			this.collection = collection;
			this.pid = this.collection.pid;
			var $li = $('#' + safe_pid(this.pid));
			
					   
			this.state = CLOSED;
			this.image = (typeof CLOSED_ICON == 'undefined') ? null : CLOSED_ICON;

			var self = this;
			this.$dom = $li.find('.collection-control').first();
			// don't render root
            if (typeof ROOT_PID == 'undefined' || ROOT_PID != this.pid) {
				self.render();
            }
		},

		render: function (state) {
			// log ("RENDER - widget for " + this.pid);
			state = (typeof state != 'undefined') ? CLOSED : state;

			if (state) {
				this.set_state(state)
			}

			if (this.collection.children.length > 0) {
				var self = this;
				this.$dom
					.html (this.image)
					.click (function (event) {
						event.preventDefault();
						self.toggle()
					})
					.mouseover (function (event) {
						$(event.target).closest(".collection-control").addClass('over');
					})
					.mouseout (function (event) {
						$(event.target).closest(".collection-control").removeClass('over');
					})
			}
		},

		toggle: function () {
//			log (this.collection.pid + ' TOGGLE - my state is: ' + this.state);
			if (this.state == OPEN) {
				this.set_state (CLOSED)
			}
			else {
				this.set_state (OPEN)
			}
		},
		set_state: function(state) {
//			log ("set_state - " + state);
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
			this.has_children = this.children.length > 0;
			this.$dom = $('li#' + safe_pid(this.pid));
			this.$body = this.$dom.find('.collection-body').first();
			this.controller = new CollectionControlWidget(this);

			this.effect_duration = 1000;  // think shadeUp duration

		},
		report: function (data) {
			return "collection " + this.data.title;
		},

		close: function () {
			this.$body.slideUp(this.effect_duration);
		},

		open: function () {
			this.$body.slideDown(this.effect_duration);
		},
		
		toggle: function () {

			if (this.$body.is(":visible"))
				this.close();
			else
				this.open();
		},
		more_description: function () {
			$desc = this.$dom.find('.description').first();
			$desc.html(this.data.description);
			var self = this;
			$desc.append($t('a')
						 .attr('href', '#')
						 .addClass("less-link")
						 .html ("less")
						 .click (function (event) {
							 event.preventDefault();
							 self.less_description();
						 }))
		},
		less_description: function () {
			$desc = this.$dom.find('.description').first();
			$desc.html(this.data.description.trimToLength(200));
			var self = this;
			$desc.append($t('a')
						 .attr('href', '#')
						 .addClass("more-link")
						 .html ("more")
						 .click (function (event) {
							 event.preventDefault();
							 self.more_description();
						 }))
		},
			

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

			if (typeof ROOT_PID != 'undefined') {
				this.root = this.collections[ROOT_PID];
				// log (' children: ' + this.root.children.length);
			}

		},
		get_collection:function (pid) {
			return this.collections[pid];
		},
		
		report: function () {
			log ("collection manager has " + Object.keys(this.data).length + " collections");
		},
	});	
	
	function decorate (initialized) {
		var initialized = (typeof initialized == 'undefined') ? false : initialized;
		log ("DECORATE - initialized is " + initialized);

		if (typeof COLLECTION_MANAGER != "defined") {
			log ('COLLECTION_MANAGER is defined')
		}		
		$.ajax ({
			'url': '/jlo/get/collections',
			'success': function (response) {
				COLLECTION_MANAGER = new CollectionManager(response.data);
				COLLECTION_MANAGER.report();

				if (initialized) {
					log ("ALREADY INITIALIZED")
				} else {

					// CLICK HANDLER
					var	node_selector = '.jlo-collection';
					$(node_selector).each (function (i, node) {
						var $node = $(node);
						var events = jQuery._data( node, "events" );
						if (typeof events != 'undefined') {
/*							log (events);
							$(events)[0].each (function (i, event) {
								log ("- " + $(event)[0].type);
							});
*/
							log ('bailing');
							return;
						}

						$node.mouseover(function (event) {
							$('.jlo-collection').removeClass('over');
							$(event.target).closest('.jlo-collection').addClass('over');
						});

						$node.click (function (event) {
							var collection = get_collection_for_el(event.target);
							var go_to_landing_page = true;
							if (collection) {
								// log ("collection: " + collection.pid + " - " + collection.children.length);				
								if (!go_to_landing_page && collection.has_children) {
									event.preventDefault()
									collection.controller.toggle();
									return false;
								} else {
									// href = "https://stage.ucar.dgicloud.com/islandora/object/" + collection.pid;
									// href = '/islandora/search/?type=dismax&collection=' + collection.pid;
									href = "https://stage.ucar.dgicloud.com/islandora/object/" + collection.pid;
									href += "?collection=" + collection.pid;  // collection param enables referrer to indicate collection
									window.location = href;
									return false;
								}
							} else {
								log ("could not find collection");
							}
						})  // end of node click

					})
				}
				/** Description MORE links
					These present a "more" link for truncated descriptions.
					The handler calls the # more_description() method of the Collection class.
				*/
				var description_selector = '.jlo-collection .collection-head .description';
				log ("doing more links ..." + $(description_selector).size());
				$(description_selector).each(function (i, el) {
					var $el = $(el);
					var current = $el.html ();
					var truncated = current.trimToLength(200);
					if (truncated != current) {
						$el.html(truncated);
						$el.append($t('a')
								   .addClass('more-link')
								   .attr('href',"#")
								   .html('more')
								   .click (function (event) {
									   event.preventDefault();
									   var pid = real_pid($(event.target).closest('li').attr('id'));
									   var collection = COLLECTION_MANAGER.get_collection(pid);
									   collection.more_description();
								   }))
					}
				});

				// USE REFERRER to reveal last node
				// collection param is REQUIRED
				// we added collection param to landing in browser
				//log ("REFERRER: " + document.referrer);
				var REFERRER_PARAM = $.parseParams(document.referrer);

				var coll_pid = (typeof PARAMS['collection'] != 'undefined') ? PARAMS['collection'] :
					(typeof REFERRER_PARAM['collection'] != 'undefined') ? REFERRER_PARAM['collection'] : null;
				
				if (coll_pid != null) {
					var collection = get_collection(coll_pid)
					while (collection) {
						// don't open the current collection
						if (collection.pid == coll_pid) {
							collection.$dom.addClass('current');
						} else {
							collection.controller.set_state(OPEN);
						}
						var $node = collection.$dom.parent().closest('li');
						if (typeof $node[0] == 'undefined') {
							break;
						}
						collection = get_collection(real_pid($node.attr('id')))
					};

					// scroll
					var selector = '#'+safe_pid(coll_pid);
					setTimeout (function () {
						$('html, body').animate({
							scrollTop: ($(selector).offset().top)
						},4000);
					}, 500);
				}
			}
		})
	}

	window.decorate_collections = decorate;
	decorate();
	
}(jQuery));



