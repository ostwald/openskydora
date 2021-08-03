<script>

function log (s) {
    if (window.console)
        console.log (s);
}

var pid = "<?php echo $pid ?>";
log ("hello from the template! " + pid);


(function ($) {

    // (VERIFY) timeout seems to be necessary, even if it is for 0 millis ...                                                                                             
    setTimeout (function () {
        // initialize from DOM, do DOM manipulate, etc 
        log ('hello from loo');
        if (pid) {
            $('#edit-pid').val(pid);
            log ('assigned pid');
        }

        $('div.form-item.form-type-textfield.form-item-pid').css ({
                'display':'inline'
                                                            });
                
    }, 0);
}(jQuery));

</script>

<div id="wrapper-description">
<!-- This is the jlo-wrapper template -->
</div>

<h2>Checksums</h2>
<?php
    $block = module_invoke('jlo', 'block_view', 'jlo_checksum_input_block');
    print render($block['content']);

if ($pid) {
    $block = module_invoke('jlo', 'block_view', 'jlo_checksum_results_block');
    print render($block['content']);
}

?>


<script>

(function ($) {


    // Sample form submission
    $('#jlo-input-form').submit (function (event) {
        $('#jlo-checksum-results').hide();
            var pid = $("#edit-pid").val();
            $("#edit-pid").val(pid.trim());
            log ("form submitted: " + pid);
            if (!pid) {
                alert ("please enter an PID");
                return false;
            }
    });


}(jQuery));

</script>
