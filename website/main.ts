import gsap from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

document.body.classList.add('js-ready');

// Hero Text Entrance
gsap.from('.reveal-hero', {
    y: 40,
    opacity: 0,
    duration: 1.2,
    stagger: 0.2,
    ease: 'power4.out'
});

// Sticky Vision Reveal
gsap.to('#vision-text', {
    scrollTrigger: {
        trigger: '.sticky-container',
        start: 'top top',
        end: 'bottom bottom',
        scrub: 1,
    },
    scale: 1.1,
    opacity: 0,
});

// Image Reveal Scale-up
gsap.to('#hero-img-reveal', {
    scrollTrigger: {
        trigger: '.sticky-container',
        start: 'bottom bottom',
        end: '+=100%',
        scrub: true,
    },
    scale: 1,
    opacity: 1,
    ease: 'power2.inOut'
});

// Scroll Reveal for other sections
const revealElements = document.querySelectorAll('.reveal');

revealElements.forEach((el) => {
    gsap.from(el, {
        scrollTrigger: {
            trigger: el,
            start: 'top 85%',
            toggleActions: 'play none none none'
        },
        y: 40,
        opacity: 0,
        duration: 1,
        ease: 'power3.out'
    });
});

// Mouse Parallax for Scattered UI
document.addEventListener('mousemove', (e) => {
    const mouseX = (e.clientX - window.innerWidth / 2) / 50;
    const mouseY = (e.clientY - window.innerHeight / 2) / 50;

    gsap.to('.scatter-item', {
        x: (i) => mouseX * (i + 1) * 0.5,
        y: (i) => mouseY * (i + 1) * 0.5,
        duration: 1,
        ease: 'power2.out'
    });

    gsap.to('.pill-bar', {
        x: mouseX * 0.2,
        y: mouseY * 0.2,
        duration: 1,
        ease: 'power2.out'
    });
});

// Smooth Scroll
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
        const href = this.getAttribute('href');
        if (!href || href === '#') return;
        
        e.preventDefault();
        const target = document.querySelector(href);
        if (target) {
            window.scrollTo({
                top: (target as HTMLElement).offsetTop - 44,
                behavior: 'smooth'
            });
        }
    });
});
