const codeBlocks = document.querySelectorAll(
  ".code-header + .highlighter-rouge"
);
const copyCodeButtons = document.querySelectorAll(".copy-code-button");

copyCodeButtons.forEach((copyCodeButton, index) => {
  const code = codeBlocks[index].innerText;
  let id;

  copyCodeButton.addEventListener("click", () => {
    window.navigator.clipboard.writeText(code);

    const img = copyCodeButton.querySelector("img");
    img.src = "/assets/images/check.svg";

    if (id) {
      clearTimeout(id);
    }

    id = setTimeout(() => {
      img.src = "/assets/images/copy.svg";
    }, 2000);
  });
});
