$(window).on('load', function(){
    var progress = localStorage.getItem("progress")
    if (progress > 0) {
        document.getElementById("progress").value = progress;
    }
    else {
        document.getElementById("progress").value = 0;
    }
});

$(".home").on('click', function() {
    var url = window.location.origin;
    window.location = url;
});

$("#up").on('click', function() {
    document.getElementById("state").innerHTML = "UP";
    localStorage.setItem("progress", "100");
});
$("#down").on('click', function() {
    document.getElementById("state").innerHTML = "DOWN";
    localStorage.setItem("progress", "0");
});
$("#stop").on('click', function() {
    document.getElementById("state").innerHTML = "STOPPED";
});