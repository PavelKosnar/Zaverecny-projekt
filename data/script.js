$(document).ready(function(){
    $(".home").on('click', function() {
        var url = window.location.origin;
        window.location = url;
    });

    $("#up").on('click', function() {
        $("#blinds-img").hide(10000);
    });
    $("#down").on('click', function() {
        $("#blinds-img").show(10000);
    });
    $("#stop").on('click', function() {
        $("#blinds-img").stop();
    });
});