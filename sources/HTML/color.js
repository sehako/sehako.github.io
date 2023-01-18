var Link = {
    setColor:function(color) {
        // var i = 0;
        // var links = document.querySelectorAll('a');
        // while(i < links.length) {
        //     links[i].style.color = color
        //     i++;
        // }
        $('a').css('color', color);
    }
}
var Body = {
    setBackground:function(color) {
        // document.querySelector('body').style.backgroundColor = color;
        $('body').css('backgroundColor', color);
    },
    setColor:function(color) {
        // document.querySelector('body').style.color = color;
        $('body').css('color', color);
    }
}

function night_day(self) {
    if (self.value == 'night') {
        Body.setBackground('black');
        Body.setColor('white');
        Link.setColor('powderblue');
        self.value = 'day';
    }
    else {
        Body.setBackground('white');
        Body.setColor('black');
        Link.setColor('blue');
        self.value = 'night';
    }
}